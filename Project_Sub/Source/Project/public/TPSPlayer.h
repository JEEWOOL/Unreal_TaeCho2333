// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSPlayer.generated.h"

UCLASS()
class PROJECT_API ATPSPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPSPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* springArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		class UCameraComponent* tpsCamComp;

	// 좌우 회전 입력 처리
	void Turn(float value);
	// 상하 회전 입력 처리
	void LookUp(float value);

	// 걷기 속도
	UPROPERTY(EditAnywhere, Category = PlayerSetting)
		float walkSpeed = 600;
	// 달리기 속도
	UPROPERTY(EditAnywhere, Category = PlayerSetting)
		float runSpeed = 800;

	// 이동방향
	FVector direction;

	//좌우 이동 입력 이벤트 처리 함수
	void InputHorizontal(float value);
	// 상하 이동 입력 이벤트 처리 함수
	void InputVertical(float value);

	// 점프 입력 이벤트 처리 함수
	void InputJump();

	// 플레이어 이동 처리
	void Move();

	// 총 스켈레탈메시
	UPROPERTY(VisibleAnywhere, Category = GunMesh)
		class USkeletalMeshComponent* gunMeshComp;

	// 총알 공장
	UPROPERTY(EditDefaultsOnly, Category = BulletFactory)
		TSubclassOf<class ABullet> bulletFactory;


	// 스나이퍼건 스태틱메시 추가
	UPROPERTY(VisibleAnywhere, Category = GunMesh)
		class UStaticMeshComponent* sniperGunComp;

	// 총알파편 효과 공장
	UPROPERTY(EditAnywhere, Category = BulletEffect)
		class UParticleSystem* bulletEffectFactory;
	UPROPERTY(EditAnywhere, Category = BulletEffect)
		class UParticleSystem* bulletEffectFactory2;

	// 일반 조준 크로스헤어UI 위젯
	UPROPERTY(EditDefaultsOnly, Category = SniperUI)
		TSubclassOf<class UUserWidget> crosshairUIFactory;
	// 크로스헤어 인스턴스
	class UUserWidget* _crosshairUI;

	// 라이플 사용 중인지 여부
	bool bUsingGrenadeGun = true;
	// 유탄총으로 변경
	void ChangeToGrenadeGun();
	// 스나이퍼건으로 변경
	void ChangeToSniperGun();

	// 총알 발사 처리 함수
	void InputFire();
	FTimerHandle timer;
	UPROPERTY()
		bool isFiring;
	void StartFire();
	void StopFire();
	void Fire();

	// 달리기 이벤트 처리 함수
	void InputRun();

	// 카메라 셰이크 블루프린트를 저장할 변수
	UPROPERTY(EditDefaultsOnly, Category = CameraMotion)
		TSubclassOf<class UCameraShakeBase> cameraShake;

	// 총알 발사 사운드
	UPROPERTY(EditAnywhere, Category = Sound)
		class USoundBase* bulletSound;

	// 현재 체력
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Health)
		int32 hp;
	// 초기 HP값
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Health)
		int32 initialHp = 10;

	// 피격 당했을 때 처리
	UFUNCTION(BlueprintCallable, Category = Health)
		void OnHitEvent();

	// 게임 오버될 때 호출될 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Health)
		void OnGameOver();

	// 총 바꿀 때 호출되는 이벤트 함수
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Health)
		void OnUsingGrenade(bool isGrenade);
};
