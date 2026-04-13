#include "MyGameMode.h"
#include "PlayerPawn.h"

AMyGameMode::AMyGameMode()
{
	DefaultPawnClass = APlayerPawn::StaticClass();
}