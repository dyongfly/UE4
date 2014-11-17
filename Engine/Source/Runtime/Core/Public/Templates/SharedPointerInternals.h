// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	SharedPointerInternals.h: Smart pointer library internal implementation
=============================================================================*/

#pragma once

/** Default behavior. */
#define	FORCE_THREADSAFE_SHAREDPTRS 0

/**
 * ESPMode is used select between either 'fast' or 'thread safe' shared pointer types.  This is
 * only used by templates at compile time to generate one code path or another.
 */
namespace ESPMode
{
	enum Type
	{
		/** Forced to be not thread-safe. */
		NotThreadSafe = 0,

		/**
		 *	Fast, doesn't ever use atomic interlocks.
		 *	Some code requires that all shared pointers are thread-safe.
		 *	It's better to change it here, instead of replacing ESPMode::Fast to ESPMode::ThreadSafe throughout the code.
		 */
		Fast = FORCE_THREADSAFE_SHAREDPTRS ? 1 : 0,

		/** Conditionally thread-safe, never spin locks, but slower */
		ThreadSafe = 1
	};
}


// Forward declarations.  Note that in the interest of fast performance, thread safety
// features are mostly turned off (Mode = ESPMode::Fast).  If you need to access your
// object on multiple threads, you should use ESPMode::ThreadSafe!
template< class ObjectType, ESPMode::Type Mode = ESPMode::Fast > class TSharedRef;
template< class ObjectType, ESPMode::Type Mode = ESPMode::Fast > class TSharedPtr;
template< class ObjectType, ESPMode::Type Mode = ESPMode::Fast > class TWeakPtr;
template< class ObjectType, ESPMode::Type Mode = ESPMode::Fast > class TSharedFromThis;



/**
 * SharedPointerInternals contains internal workings of shared and weak pointers.  You should
 * hopefully never have to use anything inside this namespace directly.
 */
namespace SharedPointerInternals
{

	// Forward declarations
	template< ESPMode::Type Mode > class FWeakReferencer;

	/** Dummy structs used internally as template arguments for typecasts */
	struct FStaticCastTag {};
	struct FConstCastTag {};

	// NOTE: The following is an Unreal extension to standard shared_ptr behavior
	struct FNullTag {};

	

	/** Proxy structure for implicitly converting raw pointers to shared/weak pointers */
	// NOTE: The following is an Unreal extension to standard shared_ptr behavior
	template< class ObjectType >
	struct FRawPtrProxy
	{
		/** The object pointer */
		ObjectType* Object;

		/** Construct implicitly from an object */
		FORCEINLINE FRawPtrProxy( ObjectType* InObject )
			: Object( InObject )
		{
		}
	};

	struct FReferenceControllerState
	{
		/** Constructor */
		FORCEINLINE FReferenceControllerState(void* InObject, void (*InDestroyObject)(void*))
			: SharedReferenceCount(1),
			  WeakReferenceCount  (1),
			  Object              (InObject),
			  DestroyObject       (InDestroyObject)
		{
		}

		// NOTE: The primary reason these reference counters are 32-bit values (and not 16-bit to save
		//       memory), is that atomic operations require at least 32-bit objects.

		/** Number of shared references to this object.  When this count reaches zero, the associated object
		    will be destroyed (even if there are still weak references!) */
		int32 SharedReferenceCount;

		/** Number of weak references to this object.  If there are any shared references, that counts as one
		   weak reference too. */
		int32 WeakReferenceCount;

		/** The object associated with this reference counter.  */
		void* Object;

		/** Destroys the object associated with this reference counter.  */
		void (*DestroyObject)(void*);

	private:
		FReferenceControllerState( FReferenceControllerState const& );
		FReferenceControllerState& operator=( FReferenceControllerState const& );
	};

	/**
	 * FReferenceController is a standalone heap-allocated object that tracks the number of references
	 * to an object referenced by TSharedRef, TSharedPtr or TWeakPtr objects.
	 *
	 * It is specialized for different threading modes.
	 */
	template< ESPMode::Type Mode >
	struct FReferenceControllerOps;

	template<>
	struct FReferenceControllerOps<ESPMode::ThreadSafe>
	{
		/** Returns the shared reference count */
		static FORCEINLINE const int32 GetSharedReferenceCount(const FReferenceControllerState* State)
		{
			// This reference count may be accessed by multiple threads
			return static_cast< int32 const volatile& >( State->SharedReferenceCount );
		}

		/** Adds a shared reference to this counter */
		static FORCEINLINE void AddSharedReference(FReferenceControllerState* State)
		{
			FPlatformAtomics::InterlockedIncrement( &State->SharedReferenceCount );
		}

		/**
		 * Adds a shared reference to this counter ONLY if there is already at least one reference
		 *
		 * @return  True if the shared reference was added successfully
		 */
		static bool ConditionallyAddSharedReference(FReferenceControllerState* State)
		{
			for( ; ; )
			{
				// Peek at the current shared reference count.  Remember, this value may be updated by
				// multiple threads.
				const int32 OriginalCount = static_cast< int32 const volatile& >( State->SharedReferenceCount );
				if( OriginalCount == 0 )
				{
					// Never add a shared reference if the pointer has already expired
					return false;
				}

				// Attempt to increment the reference count.
				const int32 ActualOriginalCount = FPlatformAtomics::InterlockedCompareExchange( &State->SharedReferenceCount, OriginalCount + 1, OriginalCount );

				// We need to make sure that we never revive a counter that has already expired, so if the
				// actual value what we expected (because it was touched by another thread), then we'll try
				// again.  Note that only in very unusual cases will this actually have to loop.
				if( ActualOriginalCount == OriginalCount )
				{
					return true;
				}
			}
		}


		/** Releases a shared reference to this counter */
		static FORCEINLINE void ReleaseSharedReference(FReferenceControllerState* State)
		{
			checkSlow( State->SharedReferenceCount > 0 );

			if( FPlatformAtomics::InterlockedDecrement( &State->SharedReferenceCount ) == 0 )
			{
				// Last shared reference was released!  Destroy the referenced object.
				State->DestroyObject(State->Object);

				// No more shared referencers, so decrement the weak reference count by one.  When the weak
				// reference count reaches zero, this object will be deleted.
				ReleaseWeakReference(State);
			}
		}


		/** Adds a weak reference to this counter */
		static FORCEINLINE void AddWeakReference(FReferenceControllerState* State)
		{
			FPlatformAtomics::InterlockedIncrement( &State->WeakReferenceCount );
		}

		/** Releases a weak reference to this counter */
		static void ReleaseWeakReference(FReferenceControllerState* State)
		{
			checkSlow( State->WeakReferenceCount > 0 );

			if( FPlatformAtomics::InterlockedDecrement( &State->WeakReferenceCount ) == 0 )
			{
				// No more references to this reference count.  Destroy it!
				delete State;
			}
		}
	};

	template<>
	struct FReferenceControllerOps<ESPMode::NotThreadSafe>
	{
		/** Returns the shared reference count */
		static FORCEINLINE const int32 GetSharedReferenceCount(const FReferenceControllerState* State)
		{
			return State->SharedReferenceCount;
		}

		/** Adds a shared reference to this counter */
		static FORCEINLINE void AddSharedReference(FReferenceControllerState* State)
		{
			++State->SharedReferenceCount;
		}

		/**
		 * Adds a shared reference to this counter ONLY if there is already at least one reference
		 *
		 * @return  True if the shared reference was added successfully
		 */
		static bool ConditionallyAddSharedReference(FReferenceControllerState* State)
		{
			if( State->SharedReferenceCount == 0 )
			{
				// Never add a shared reference if the pointer has already expired
				return false;
			}

			++State->SharedReferenceCount;
			return true;
		}


		/** Releases a shared reference to this counter */
		static FORCEINLINE void ReleaseSharedReference(FReferenceControllerState* State)
		{
			checkSlow( State->SharedReferenceCount > 0 );

			if( --State->SharedReferenceCount == 0 )
			{
				// Last shared reference was released!  Destroy the referenced object.
				State->DestroyObject(State->Object);

				// No more shared referencers, so decrement the weak reference count by one.  When the weak
				// reference count reaches zero, this object will be deleted.
				ReleaseWeakReference(State);
			}
		}


		/** Adds a weak reference to this counter */
		static FORCEINLINE void AddWeakReference(FReferenceControllerState* State)
		{
			++State->WeakReferenceCount;
		}

		/** Releases a weak reference to this counter */
		static void ReleaseWeakReference(FReferenceControllerState* State)
		{
			checkSlow( State->WeakReferenceCount > 0 );

			if( --State->WeakReferenceCount == 0 )
			{
				// No more references to this reference count.  Destroy it!
				delete State;
			}
		}
	};

	/** Type-erasing function deleting objects of a given type via a common pointer type (void*) */
	template <typename Type>
	void DestroyObject(void* Object)
	{
		delete (Type*)Object;
	}

	/**
	 * FSharedReferencer is a wrapper around a pointer to a reference controller that is used by either a
	 * TSharedRef or a TSharedPtr to keep track of a referenced object's lifetime
	 */
	template< ESPMode::Type Mode >
	class FSharedReferencer
	{
		typedef FReferenceControllerOps<Mode> TOps;

	public:

		/** Constructor for an empty shared referencer object */
		FORCEINLINE FSharedReferencer()
			: ReferenceController( NULL )
		{
		}

		
		/** Constructor that counts a single reference to the specified object */
		template< class OtherType > 
		inline explicit FSharedReferencer( OtherType* InObject )
			: ReferenceController( new FReferenceControllerState( InObject, &DestroyObject<OtherType> ) )
		{
		}

		
		/** Copy constructor creates a new reference to the existing object */
		FORCEINLINE FSharedReferencer( FSharedReferencer const& InSharedReference )
			: ReferenceController( InSharedReference.ReferenceController )
		{
			// If the incoming reference had an object associated with it, then go ahead and increment the
			// shared reference count
			if( ReferenceController != NULL )
			{
				TOps::AddSharedReference(ReferenceController);
			}
		}

		/** Move constructor creates no new references */
		FORCEINLINE FSharedReferencer( FSharedReferencer&& InSharedReference )
			: ReferenceController( InSharedReference.ReferenceController )
		{
			InSharedReference.ReferenceController = NULL;
		}


		/** Creates a shared referencer object from a weak referencer object.  This will only result
		    in a valid object reference if the object already has at least one other shared referencer. */
		FSharedReferencer( FWeakReferencer< Mode > const& InWeakReference )
			: ReferenceController( InWeakReference.ReferenceController )
		{
			// If the incoming reference had an object associated with it, then go ahead and increment the
			// shared reference count
			if( ReferenceController != NULL )
			{
				// Attempt to elevate a weak reference to a shared one.  For this to work, the object this
				// weak counter is associated with must already have at least one shared reference.  We'll
				// never revive a pointer that has already expired!
				if( !TOps::ConditionallyAddSharedReference(ReferenceController) )
				{
					ReferenceController = NULL;
				}
			}
		}


		/** Destructor */
		FORCEINLINE ~FSharedReferencer()
		{
			if( ReferenceController != NULL )
			{
				// Tell the reference counter object that we're no longer referencing the object with
				// this shared pointer
				TOps::ReleaseSharedReference(ReferenceController);
			}
		}

		/** Assignment operator adds a reference to the assigned object.  If this counter was previously
		    referencing an object, that reference will be released. */
		inline FSharedReferencer& operator=( FSharedReferencer const& InSharedReference )
		{
			// Make sure we're not be reassigned to ourself!
			auto NewReferenceController = InSharedReference.ReferenceController;
			if( NewReferenceController != ReferenceController )
			{
				// First, add a shared reference to the new object
				if( NewReferenceController != NULL )
				{
					TOps::AddSharedReference(NewReferenceController);
				}

				// Release shared reference to the old object
				if( ReferenceController != NULL )
				{
					TOps::ReleaseSharedReference(ReferenceController);
				}

				// Assume ownership of the assigned reference counter
				ReferenceController = NewReferenceController;
			}

			return *this;
		}

		/** Move assignment operator adds no references to the assigned object.  If this counter was previously
		    referencing an object, that reference will be released. */
		inline FSharedReferencer& operator=( FSharedReferencer&& InSharedReference )
		{
			// Make sure we're not be reassigned to ourself!
			auto NewReferenceController = InSharedReference.ReferenceController;
			auto OldReferenceController = ReferenceController;
			if( NewReferenceController != OldReferenceController )
			{
				// Assume ownership of the assigned reference counter
				InSharedReference.ReferenceController = NULL;
				ReferenceController                   = NewReferenceController;

				// Release shared reference to the old object
				if( OldReferenceController != NULL )
				{
					TOps::ReleaseSharedReference(OldReferenceController);
				}
			}

			return *this;
		}


		/**
		 * Tests to see whether or not this shared counter contains a valid reference
		 *
		 * @return  True if reference is valid
		 */
		FORCEINLINE const bool IsValid() const
		{
			return ReferenceController != NULL;
		}


		/**
		 * Returns the number of shared references to this object (including this reference.)
		 *
		 * @return  Number of shared references to the object (including this reference.)
		 */
		FORCEINLINE const int32 GetSharedReferenceCount() const
		{
			return ReferenceController != NULL ? TOps::GetSharedReferenceCount(ReferenceController) : 0;
		}


		/**
		 * Returns true if this is the only shared reference to this object.  Note that there may be
		 * outstanding weak references left.
		 *
		 * @return  True if there is only one shared reference to the object, and this is it!
		 */
		FORCEINLINE const bool IsUnique() const
		{
			return GetSharedReferenceCount() == 1;
		}


	private:

 		// Expose access to ReferenceController to FWeakReferencer
		template< ESPMode::Type OtherMode > friend class FWeakReferencer;


	private:

		/** Pointer to the reference controller for the object a shared reference/pointer is referencing */
		FReferenceControllerState* ReferenceController;
	};



	/**
	 * FWeakReferencer is a wrapper around a pointer to a reference controller that is used
	 * by a TWeakPtr to keep track of a referenced object's lifetime.
	 */
	template< ESPMode::Type Mode >
	class FWeakReferencer
	{
		typedef FReferenceControllerOps<Mode> TOps;

	public:

		/** Default constructor with empty counter */
		FORCEINLINE FWeakReferencer()
			: ReferenceController( NULL )
		{
		}


		/** Construct a weak referencer object from another weak referencer */
		FORCEINLINE FWeakReferencer( FWeakReferencer const& InWeakRefCountPointer )
			: ReferenceController( InWeakRefCountPointer.ReferenceController )
		{
			// If the weak referencer has a valid controller, then go ahead and add a weak reference to it!
			if( ReferenceController != NULL )
			{
				TOps::AddWeakReference(ReferenceController);
			}
		}


		/** Construct a weak referencer object from an rvalue weak referencer */
		FORCEINLINE FWeakReferencer( FWeakReferencer&& InWeakRefCountPointer )
			: ReferenceController( InWeakRefCountPointer.ReferenceController )
		{
			InWeakRefCountPointer.ReferenceController = NULL;
		}


		/** Construct a weak referencer object from a shared referencer object */
		FORCEINLINE FWeakReferencer( FSharedReferencer< Mode > const& InSharedRefCountPointer )
			: ReferenceController( InSharedRefCountPointer.ReferenceController )
		{
			// If the shared referencer had a valid controller, then go ahead and add a weak reference to it!
			if( ReferenceController != NULL )
			{
				TOps::AddWeakReference(ReferenceController);
			}
		}


		/** Destructor */
		FORCEINLINE ~FWeakReferencer()
		{
			if( ReferenceController != NULL )
			{
				// Tell the reference counter object that we're no longer referencing the object with
				// this weak pointer
				TOps::ReleaseWeakReference(ReferenceController);
			}
		}

		
		/** Assignment operator from a weak referencer object.  If this counter was previously referencing an
		    object, that reference will be released. */
		FORCEINLINE FWeakReferencer& operator=( FWeakReferencer const& InWeakReference )
		{
			AssignReferenceController( InWeakReference.ReferenceController );

			return *this;
		}


		/** Assignment operator from an rvalue weak referencer object.  If this counter was previously referencing an
		    object, that reference will be released. */
		FORCEINLINE FWeakReferencer& operator=( FWeakReferencer&& InWeakReference )
		{
			auto OldReferenceController         = ReferenceController;
			ReferenceController                 = InWeakReference.ReferenceController;
			InWeakReference.ReferenceController = NULL;
			if( OldReferenceController != NULL )
			{
				TOps::ReleaseWeakReference(OldReferenceController);
			}

			return *this;
		}


		/** Assignment operator from a shared reference counter.  If this counter was previously referencing an
		   object, that reference will be released. */
		FORCEINLINE FWeakReferencer& operator=( FSharedReferencer< Mode > const& InSharedReference )
		{
			AssignReferenceController( InSharedReference.ReferenceController );

			return *this;
		}


		/**
		 * Tests to see whether or not this weak counter contains a valid reference
		 *
		 * @return  True if reference is valid
		 */
		FORCEINLINE const bool IsValid() const
		{
			return ReferenceController != NULL && TOps::GetSharedReferenceCount(ReferenceController) > 0;
		}


	private:

		/** Assigns a new reference controller to this counter object, first adding a reference to it, then
		    releasing the previous object. */
		inline void AssignReferenceController( FReferenceControllerState* NewReferenceController )
		{
			// Only proceed if the new reference counter is different than our current
			if( NewReferenceController != ReferenceController )
			{
				// First, add a weak reference to the new object
				if( NewReferenceController != NULL )
				{
					TOps::AddWeakReference(NewReferenceController);
				}

				// Release weak reference to the old object
				if( ReferenceController != NULL )
				{
					TOps::ReleaseWeakReference(ReferenceController);
				}

				// Assume ownership of the assigned reference counter
				ReferenceController = NewReferenceController;
			}
		}


	private:

 		// Expose access to ReferenceController to FSharedReferencer
		template< ESPMode::Type OtherMode > friend class FSharedReferencer;


	private:

		/** Pointer to the reference controller for the object a TWeakPtr is referencing */
		FReferenceControllerState* ReferenceController;
	};


	/** Templated helper function (const) that creates a shared pointer from an object instance */
	template< class SharedPtrType, class ObjectType, class OtherType, ESPMode::Type Mode >
	FORCEINLINE void EnableSharedFromThis( TSharedPtr< SharedPtrType, Mode > const* InSharedPtr, ObjectType const* InObject, TSharedFromThis< OtherType, Mode > const* InShareable )
	{
		if( InShareable != NULL )
		{
			InShareable->UpdateWeakReferenceInternal( InSharedPtr, const_cast< ObjectType* >( InObject ) );
		}
	}


	/** Templated helper function that creates a shared pointer from an object instance */
	template< class SharedPtrType, class ObjectType, class OtherType, ESPMode::Type Mode >
	FORCEINLINE void EnableSharedFromThis( TSharedPtr< SharedPtrType, Mode >* InSharedPtr, ObjectType const* InObject, TSharedFromThis< OtherType, Mode > const* InShareable )
	{
		if( InShareable != NULL )
		{
			InShareable->UpdateWeakReferenceInternal( InSharedPtr, const_cast< ObjectType* >( InObject ) );
		}
	}


	/** Templated helper function (const) that creates a shared reference from an object instance */
	template< class SharedRefType, class ObjectType, class OtherType, ESPMode::Type Mode >
	FORCEINLINE void EnableSharedFromThis( TSharedRef< SharedRefType, Mode > const* InSharedRef, ObjectType const* InObject, TSharedFromThis< OtherType, Mode > const* InShareable )
	{
		if( InShareable != NULL )
		{
			InShareable->UpdateWeakReferenceInternal( InSharedRef, const_cast< ObjectType* >( InObject ) );
		}
	}


	/** Templated helper function that creates a shared reference from an object instance */
	template< class SharedRefType, class ObjectType, class OtherType, ESPMode::Type Mode >
	FORCEINLINE void EnableSharedFromThis( TSharedRef< SharedRefType, Mode >* InSharedRef, ObjectType const* InObject, TSharedFromThis< OtherType, Mode > const* InShareable )
	{
		if( InShareable != NULL )
		{
			InShareable->UpdateWeakReferenceInternal( InSharedRef, const_cast< ObjectType* >( InObject ) );
		}
	}


	/** Templated helper catch-all function, accomplice to the above helper functions */
	FORCEINLINE void EnableSharedFromThis( ... )
	{
	}


}	// namespace SharedPointerInternals



