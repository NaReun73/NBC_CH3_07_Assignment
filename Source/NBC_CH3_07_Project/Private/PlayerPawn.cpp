
#include "PlayerPawn.h"
#include "Components/SphereComponent.h"
#include "MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

APlayerPawn::APlayerPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	// Sphere컴포넌트 생성
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	// Sphere 컴포넌트를 루트 컴포넌트로 지정
	SetRootComponent(SphereComp);
	// SimulatePhysics를 false로 설정
	SphereComp->SetSimulatePhysics(false);

	// 스켈레탈 메쉬 생성
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	// 스켈레탈 메쉬를 루트 컴포넌트 (SphereComp)에 부착
	SkeletalMeshComp->SetupAttachment(SphereComp);
	// SimulatePhysics를 false로 설정
	SkeletalMeshComp->SetSimulatePhysics(false);

	// 스프링 암 생성
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	// 스프링 암을 루트 컴포넌트 (SphereComp)에 부착
	SpringArmComp->SetupAttachment(SphereComp);
	// 캐릭터와 카메라 사이의 거리 기본값 300으로 설정
	SpringArmComp->TargetArmLength = 300.0f;
	// 컨트롤러 회전에 따라 스프링 암도 회전하도록 설정
	SpringArmComp->bUsePawnControlRotation = true;

	// 카메라 컴포넌트 생성
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	// 스프링 암의 소켓 위치에 카메라를 부착
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	// 카메라는 스프링 암의 회전을 따르므로 PawnControlRotation은 꺼둠
	CameraComp->bUsePawnControlRotation = false;

	// 이동 컴포넌트 생성 (부착할 필요 없이 생성만 하면 자동으로 Root를 제어합니다)
	//MovementComp = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComp"));
}

void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerPawn::Move
				);
			}

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerPawn::Look
				);
			}
		}
	}

}

void APlayerPawn::Move(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector2D MoveInput = value.Get<FVector2D>();

	if (MoveInput.IsNearlyZero())	return;

	float moveSpeed = 500.0f;

	FVector DeltaLocation(MoveInput.X * moveSpeed * GetWorld()->GetDeltaSeconds(),
						  MoveInput.Y * moveSpeed * GetWorld()->GetDeltaSeconds(),
						  0.0f);

	AddActorLocalOffset(DeltaLocation, true);

	/*if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(GetActorForwardVector(), MoveInput.X);

	}

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(GetActorRightVector(), MoveInput.Y);
	}*/
}

void APlayerPawn::Look(const FInputActionValue& value)
{
	FVector2D LookInput = value.Get<FVector2D>();

	float Speed = 50.0f;

	FRotator DeltaYaw(0.0f, LookInput.X * Speed * GetWorld()->GetDeltaSeconds(), 0.0f);
	AddActorLocalRotation(DeltaYaw);

	/*float YawAmount = LookInput.X * Speed * GetWorld()->GetDeltaSeconds();
	FRotator CurrentActorRotation = GetActorRotation();
	CurrentActorRotation.Yaw += YawAmount;
	SetActorRotation(CurrentActorRotation);*/

	if (SpringArmComp)
	{
		FRotator DeltaPitch(-LookInput.Y * Speed * GetWorld()->GetDeltaSeconds(), 0.0f, 0.0f);
		SpringArmComp->AddLocalRotation(DeltaPitch);

		/*float PitchAmount = -LookInput.Y * Speed * GetWorld()->GetDeltaSeconds();
		FRotator NewSpringArmRotation = SpringArmComp->GetRelativeRotation();
		float TargetPitch = NewSpringArmRotation.Pitch + PitchAmount;

		NewSpringArmRotation.Pitch = FMath::Clamp(TargetPitch, -80.0f, 80.0f);
		SpringArmComp->AddRelativeRotation(NewSpringArmRotation);*/
	}
	

	//AddControllerYawInput(LookInput.X);
	//AddControllerPitchInput(LookInput.Y);
}