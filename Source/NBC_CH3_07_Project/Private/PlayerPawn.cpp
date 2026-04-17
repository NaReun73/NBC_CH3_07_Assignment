
#include "PlayerPawn.h"
#include "Components/SphereComponent.h"
#include "MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

APlayerPawn::APlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;

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

	// 스태틱 메쉬 (드론 적용을 위한 추가)
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComp->SetupAttachment(SphereComp);
	StaticMeshComp->SetSimulatePhysics(false);

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

	StateAir = false;
}

void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 호버링
	// 상하 이동 입력이 없으면 위치 고정
	if (FMath::IsNearlyZero(UpDownInput))
	{
		VerticalVelocity = FMath::FInterpTo(VerticalVelocity, 0.0f, DeltaTime, 2.0f);
		if (FMath::Abs(VerticalVelocity) < 1.0f) VerticalVelocity = 0.0f;
	}
	else
	{
		VerticalVelocity += (GravityStrength + (UpDownInput * 1200.0f)) * DeltaTime;
	}

	// UpDown 입력값 초기화 (초기화 안하면 계속 상승, 하강)
	UpDownInput = 0.0f;;

	// 속도 제한, 이동
	VerticalVelocity = FMath::Clamp(VerticalVelocity, -1000.0f, 1000.0f);
	FVector GravityDelta(0.0f, 0.0f, VerticalVelocity * DeltaTime);
	FHitResult HitResult;
	AddActorLocalOffset(GravityDelta, true, &HitResult);

	// 충돌하면 속도 0
	if (HitResult.IsValidBlockingHit())
	{
		if (HitResult.ImpactNormal.Z > 0.5f)
		{
			VerticalVelocity = 0.0f;
		}
	}

	if (GetActorLocation().Z <= 200.0f)
	{
		StateAir = false;
	}
	else
	{
		StateAir = true;
	}
}

void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//  Enhanced InputComponent로 캐스팅
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// IA를 가져오기 위해 현재 소유 중인 Controller를 AMyPlayerController로 캐스팅
		if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				// IA_Move 액션 키를 누르고 있는 동안 Move()호출
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerPawn::Move
				);
			}

			if (PlayerController->LookAction)
			{
				// IA_Look 액션 마우스가 움직일 때 Look() 호출
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerPawn::Look
				);
			}

			if (PlayerController->UpDownAction)
			{
				EnhancedInput->BindAction(
					PlayerController->UpDownAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerPawn::UpDown
				);
			}

			if (PlayerController->TiltAction)
			{
				EnhancedInput->BindAction(
					PlayerController->TiltAction,
					ETriggerEvent::Triggered,
					this,
					&APlayerPawn::Tilt
				);
			}
		}
	}

}

void APlayerPawn::Move(const FInputActionValue& value)
{
	// 컨트롤러가 있어야 방향 계산
	if (!Controller) return;

	// value는 Axis2D로 설정된 IA_Move의 입력값(WASD)을 담고있다
	// (x=1, y=0) 전진, (x=-1, y=0) 후진, (x=0, y=1) 오른쪽, (x=0, y=-1) 왼쪽
	const FVector2D MoveInput = value.Get<FVector2D>();

	if (MoveInput.IsNearlyZero())	return;

	float moveSpeed = 500.0f;

	// Pawn이 공중에 있는 상태면 속도 감소
	if (StateAir)
	{
		moveSpeed *= 0.4f;
	}

	FVector DeltaLocation(MoveInput.X * moveSpeed * GetWorld()->GetDeltaSeconds(),
						  MoveInput.Y * moveSpeed * GetWorld()->GetDeltaSeconds(),
						  0.0f);

	AddActorLocalOffset(DeltaLocation, true);

	/*if (!FMath::IsNearlyZero(MoveInput.X))
	{
		// 캐릭터가 바라보는 방향(정면)으로 X축 이동
		AddMovementInput(GetActorForwardVector(), MoveInput.X);
	}

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		// 캐릭터의 오른쪽 방향으로 Y축 이동
		AddMovementInput(GetActorRightVector(), MoveInput.Y);
	}*/
}

void APlayerPawn::Look(const FInputActionValue& value)
{
	// 마우스의 X, Y 움직임을 2D값으로 가져옴
	FVector2D LookInput = value.Get<FVector2D>();

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	float LookSpeed = 50.0f;

	// 각도 제한 없을 떄
	//FRotator DeltaYaw(0.0f, LookInput.X * Speed * DeltaTime, 0.0f);
	//AddActorLocalRotation(DeltaYaw);

	// 각도 제한 있을 때
	float YawAmount = LookInput.X * LookSpeed * DeltaTime;
	FRotator CurrentActorRotation = GetActorRotation();
	CurrentActorRotation.Yaw += LookInput.X * LookSpeed * DeltaTime;
	SetActorRotation(CurrentActorRotation);

	// 드론일 경우
	FRotator CurrentRot = GetActorRotation();
	float NewPitch = CurrentRot.Pitch + (LookInput.Y * LookSpeed * DeltaTime);
	CurrentRot.Pitch = FMath::Clamp(NewPitch, -45.0f, 45.0f);
	SetActorRotation(CurrentRot);

	// 사람일 경우
	//if (SpringArmComp)
	//{
	//	// 각도 제한 없을 때
	//	//FRotator DeltaPitch(-LookInput.Y * Speed * DeltaTime, 0.0f, 0.0f);
	//	//SpringArmComp->AddLocalRotation(DeltaPitch);

	//	// 각도 제한 있을 때
	//	float PitchAmount = -LookInput.Y * Speed * DeltaTime;
	//	FRotator NewSpringArmRotation = SpringArmComp->GetRelativeRotation();
	//	float TargetPitch = NewSpringArmRotation.Pitch + PitchAmount;

	//	NewSpringArmRotation.Pitch = FMath::Clamp(TargetPitch, -80.0f, 80.0f);
	//	SpringArmComp->SetRelativeRotation(NewSpringArmRotation);
	//}
	
	// 강의에서 나온 구현방법
	// X는 좌우회전 (Yaw), Y는 상하 회전 (Pitch)
	//AddControllerYawInput(LookInput.X);
	//AddControllerPitchInput(LookInput.Y);
}

void APlayerPawn::UpDown(const FInputActionValue& value)
{
	if (!Controller) return;

	// 스페이스를 누르면 1, 쉬프트를 누르면 -1
	UpDownInput = value.Get<float>();

	if (FMath::IsNearlyZero(UpDownInput))	return;

	// 중력이 있을 때는 중력보다 큰 값을 넣어줘야 상하 움직임 가능
	float Speed = 1300.0f;

	// 중력 계산
	VerticalVelocity += UpDownInput * Speed * GetWorld()->GetDeltaSeconds();

	// 중력 X
	/*FVector DeltaLocation(0.0f, 0.0f, UpDownInput * Speed * GetWorld()->GetDeltaSeconds());
	AddActorLocalOffset(DeltaLocation, true);*/

}

void APlayerPawn::Tilt(const FInputActionValue& value)
{
	if (!Controller)	return;

	float TiltInput = value.Get<float>();

	if (FMath::IsNearlyZero(TiltInput)) return;

	float TiltSpeed = 200.0f;

	FRotator CurrentRot = GetActorRotation();
	float NewRoll = CurrentRot.Roll + (TiltInput * TiltSpeed * GetWorld()->GetDeltaSeconds());
	CurrentRot.Roll = FMath::Clamp(NewRoll, -45.0f, 45.0f);

	SetActorRotation(CurrentRot);
}