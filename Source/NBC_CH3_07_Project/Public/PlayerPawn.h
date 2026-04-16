#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawn.generated.h"

class USphereComponent;
class USpringArmComponent;
class UCameraComponent;
// Enhanced Input에서 액션 값을 받을 때 사용하는 구조체
struct FInputActionValue;
// Pawn 클래스 무브 컴포넌트
//class UFloatingPawnMovement;

UCLASS()
class NBC_CH3_07_PROJECT_API APlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	APlayerPawn();

protected:
	
	// 콜리전 (루트 컴포넌트)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* SphereComp;

	// 스켈레탈 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* SkeletalMeshComp;

	// 스태틱 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComp;

	// 스프링 암
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	// 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	// Pawn 무브 컴포넌트
	//UPROPERTY(VisibleAnywhere, Category = "Components")
	//UFloatingPawnMovement* MovementComp;

	// 입력 바인딩을 처리할 함수
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// IA_Move와 IA_Look 등을 처리할 함수 원형
	// Enhanced Input에서 액션 값은 FInputActionValue로 전달됩니다.
	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	UFUNCTION()
	void UpDown(const FInputActionValue& value);
	UFUNCTION()
	void Tilt(const FInputActionValue& value);

	// 중력 구현
	float VerticalVelocity = 0.0f;
	float GravityStrength = -980.0f;

	// 호버링을 구현하기 위해 변수로 뺌
	float UpDownInput;
public:	
	virtual void Tick(float DeltaTime) override;

};
