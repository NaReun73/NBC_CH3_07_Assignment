#include "MyGameMode.h"
#include "MyCharacter.h"

AMyGameMode::AMyGameMode()
{
	DefaultPawnClass = AMyCharacter::StaticClass();
}