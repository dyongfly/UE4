// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

class FJsonSerializer
{
public:

	template <class CharType>
	static bool Deserialize( const TSharedRef< TJsonReader<CharType> >& Reader, TArray< TSharedPtr<FJsonValue> >& OutArray )
	{
		StackState State;
		if ( !Deserialize( Reader, /*OUT*/State ) )
		{
			return false;
		}

		if ( State.Object.IsValid() )
		{
			return false;
		}

		OutArray = State.Array;
		return true;
	}

	template <class CharType>
	static bool Deserialize( const TSharedRef< TJsonReader<CharType> >& Reader, TSharedPtr<FJsonObject>& OutObject )
	{
		StackState State;
		if ( !Deserialize( Reader, /*OUT*/State ) )
		{
			return false;
		}

		if ( !State.Object.IsValid() )
		{
			return false;
		}

		OutObject = State.Object;
		return true;
	}

	template <class CharType, class PrintPolicy >
	static bool Serialize( const TArray< TSharedPtr<FJsonValue> >& Array, const TSharedRef< TJsonWriter< CharType, PrintPolicy > >& Writer )
	{
		TSharedRef< FElement > StartingElement = MakeShareable( new FElement( Array ) );
		return FJsonSerializer::Serialize<CharType, PrintPolicy>( StartingElement, Writer );
	}

	template <class CharType, class PrintPolicy >
	static bool Serialize( const TSharedRef< FJsonObject >& Object, const TSharedRef< TJsonWriter< CharType, PrintPolicy > >& Writer )
	{
		TSharedRef< FElement > StartingElement = MakeShareable( new FElement( Object ) );
		return FJsonSerializer::Serialize<CharType, PrintPolicy>( StartingElement, Writer );
	}

	template <class CharType, class PrintPolicy >
	static bool Serialize( const TSharedPtr<FJsonValue >& Value, const FString& Identifier, const TSharedRef< TJsonWriter< CharType, PrintPolicy > >& Writer )
	{
		TSharedRef< FElement > StartingElement = MakeShareable( new FElement( Identifier, Value ) );
		return FJsonSerializer::Serialize<CharType, PrintPolicy>( StartingElement, Writer );
	}
	

private:

	struct StackState
	{
		EJson::Type Type;
		FString Identifier;
		TArray< TSharedPtr<FJsonValue> > Array;
		TSharedPtr< FJsonObject > Object;
	};

	struct FElement
	{
		FElement( const TSharedPtr< FJsonValue >& InValue )
			: Identifier()
			, Value( InValue )
			, HasBeenProcessed( false )
		{

		}

		FElement( const TSharedRef< FJsonObject >& Object )
			: Identifier()
			, Value( MakeShareable( new FJsonValueObject( Object ) ) )
			, HasBeenProcessed( false )
		{

		}

		FElement( const TArray< TSharedPtr<FJsonValue> >& Array )
			: Identifier()
			, Value( MakeShareable( new FJsonValueArray( Array ) ) )
			, HasBeenProcessed( false )
		{

		}

		FElement( const FString& InIdentifier, const TSharedPtr< FJsonValue >& InValue )
			: Identifier( InIdentifier )
			, Value( InValue )
			, HasBeenProcessed( false )
		{

		}

		FString Identifier;
		TSharedPtr< FJsonValue > Value;
		bool HasBeenProcessed;
	};

private:

	template <class CharType>
	static bool Deserialize( const TSharedRef< TJsonReader<CharType> >& Reader, StackState& OutStackState )
	{
		TArray< TSharedRef< StackState > > ScopeStack; 
		TSharedPtr< StackState > CurrentState;

		TSharedPtr<FJsonValue> NewValue;
		EJsonNotation::Type Notation;
		while( Reader->ReadNext( Notation ) )
		{
			FString Identifier = Reader->GetIdentifier();
			NewValue.Reset();

			switch( Notation )
			{
			case EJsonNotation::ObjectStart:
				{
					if ( CurrentState.IsValid() )
					{
						ScopeStack.Push( CurrentState.ToSharedRef() );
					}

					CurrentState = MakeShareable( new StackState() );
					CurrentState->Type = EJson::Object;
					CurrentState->Identifier = Identifier;
					CurrentState->Object = MakeShareable( new FJsonObject() );
				}
				break;

			case EJsonNotation::ObjectEnd:
				{
					if ( ScopeStack.Num() > 0 )
					{
						Identifier = CurrentState->Identifier;
						NewValue = MakeShareable( new FJsonValueObject( CurrentState->Object ) );
						CurrentState = ScopeStack.Pop();
					}
				}
				break;

			case EJsonNotation::ArrayStart:
				{
					if ( CurrentState.IsValid() )
					{
						ScopeStack.Push( CurrentState.ToSharedRef() );
					}

					CurrentState = MakeShareable( new StackState() );
					CurrentState->Type = EJson::Array;
					CurrentState->Identifier = Identifier;
				}
				break;

			case EJsonNotation::ArrayEnd:
				{
					if ( ScopeStack.Num() > 0 )
					{
						Identifier = CurrentState->Identifier;
						NewValue = MakeShareable( new FJsonValueArray( CurrentState->Array ) );
						CurrentState = ScopeStack.Pop();
					}
				}
				break;

			case EJsonNotation::Boolean:
				NewValue = MakeShareable( new FJsonValueBoolean( Reader->GetValueAsBoolean() ) );
				break;

			case EJsonNotation::String:
				NewValue = MakeShareable( new FJsonValueString( Reader->GetValueAsString() ) );
				break;

			case EJsonNotation::Number:
				NewValue = MakeShareable( new FJsonValueNumber( Reader->GetValueAsNumber() ) );
				break;

			case EJsonNotation::Null:
				NewValue = MakeShareable( new FJsonValueNull() );
				break;

			case EJsonNotation::Error:
				return false;
				break;
			}

			if ( NewValue.IsValid() && CurrentState.IsValid() )
			{
				if ( CurrentState->Type == EJson::Object )
				{
					CurrentState->Object->SetField( Identifier, NewValue );
				}
				else
				{
					CurrentState->Array.Add( NewValue );
				}
			}
		}

		if ( !CurrentState.IsValid() || !Reader->GetErrorMessage().IsEmpty() )
		{
			return false;
		}

		OutStackState = *CurrentState.Get();
		return true;
	}

	template <class CharType, class PrintPolicy >
	static bool Serialize( const TSharedRef< FElement >& StartingElement, const TSharedRef< TJsonWriter< CharType, PrintPolicy > >& Writer )
	{
		TArray< TSharedRef< FElement > > ElementStack;
		ElementStack.Push( StartingElement );

		while( ElementStack.Num() > 0 )
		{
			TSharedRef< FElement > Element = ElementStack.Pop();
			check(Element->Value->Type != EJson::None);

			switch (Element->Value->Type)
			{
			case EJson::Number:	
				{
					if ( Element->Identifier.IsEmpty() )
					{
						Writer->WriteValue( Element->Value->AsNumber() );	
					}
					else
					{
						Writer->WriteValue( Element->Identifier, Element->Value->AsNumber() );	
					}
				}
				break;

			case EJson::Boolean:					
				{
					if ( Element->Identifier.IsEmpty() )
					{
						Writer->WriteValue( Element->Value->AsBool() );	
					}
					else
					{
						Writer->WriteValue( Element->Identifier, Element->Value->AsBool() );	
					}
				}
				break;

			case EJson::String:
				{
					if ( Element->Identifier.IsEmpty() )
					{
						Writer->WriteValue( Element->Value->AsString() );	
					}
					else
					{
						Writer->WriteValue( Element->Identifier, Element->Value->AsString() );	
					}
				}
				break;

			case EJson::Null:
				{
					if ( Element->Identifier.IsEmpty() )
					{
						Writer->WriteNull();	
					}
					else
					{
						Writer->WriteNull( Element->Identifier );	
					}
				}
				break;

			case EJson::Array:
				{
					if ( Element->HasBeenProcessed )
					{
						Writer->WriteArrayEnd();
					}
					else
					{
						Element->HasBeenProcessed = true;
						ElementStack.Push( Element );

						if ( Element->Identifier.IsEmpty() )
						{
							Writer->WriteArrayStart();
						}
						else
						{
							Writer->WriteArrayStart( Element->Identifier );
						}

						TArray< TSharedPtr<FJsonValue> > Values = Element->Value->AsArray();

						for( int Index = Values.Num() - 1; Index >= 0; --Index )
						{
							ElementStack.Push( MakeShareable( new FElement( Values[ Index ] ) ) );
						}
					}
				}
				break;

			case EJson::Object:
				{
					if ( Element->HasBeenProcessed )
					{
						Writer->WriteObjectEnd();
					}
					else
					{
						Element->HasBeenProcessed = true;
						ElementStack.Push( Element );

						if ( Element->Identifier.IsEmpty() )
						{
							Writer->WriteObjectStart();
						}
						else
						{
							Writer->WriteObjectStart( Element->Identifier );
						}

						TArray<FString> Keys; 
						TArray< TSharedPtr<FJsonValue> > Values;
						TSharedPtr< FJsonObject > ElementObject = Element->Value->AsObject();
						ElementObject->Values.GenerateKeyArray(Keys);
						ElementObject->Values.GenerateValueArray(Values);

						check(Keys.Num() == Values.Num());

						for( int Index = Values.Num() - 1; Index >= 0; --Index )
						{
							ElementStack.Push( MakeShareable( new FElement( Keys[ Index ], Values[ Index ] ) ) );
						}
					}
				}
				break;

			default: 
				UE_LOG(LogJson, Fatal,TEXT("Could not print Json Value, unrecognized type."));
			}
		}

		return Writer->Close();
	}
};