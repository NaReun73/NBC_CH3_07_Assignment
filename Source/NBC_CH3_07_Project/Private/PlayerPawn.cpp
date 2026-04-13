
#include "PlayerPawn.h"
#include "Components/SphereComponent.h"
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

}

void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

