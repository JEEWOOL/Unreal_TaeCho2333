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

	// �¿� ȸ�� �Է� ó��
	void Turn(float value);
	// ���� ȸ�� �Է� ó��
	void LookUp(float value);

	// �ȱ� �ӵ�
	UPROPERTY(EditAnywhere, Category = PlayerSetting)
		float walkSpeed = 600;
	// �޸��� �ӵ�
	UPROPERTY(EditAnywhere, Category = PlayerSetting)
		float runSpeed = 800;

	// �̵�����
	FVector direction;

	//�¿� �̵� �Է� �̺�Ʈ ó�� �Լ�
	void InputHorizontal(float value);
	// ���� �̵� �Է� �̺�Ʈ ó�� �Լ�
	void InputVertical(float value);

	// ���� �Է� �̺�Ʈ ó�� �Լ�
	void InputJump();

	// �÷��̾� �̵� ó��
	void Move();

	// �� ���̷�Ż�޽�
	UPROPERTY(VisibleAnywhere, Category = GunMesh)
		class USkeletalMeshComponent* gunMeshComp;

	// �Ѿ� ����
	UPROPERTY(EditDefaultsOnly, Category = BulletFactory)
		TSubclassOf<class ABullet> bulletFactory;


	// �������۰� ����ƽ�޽� �߰�
	UPROPERTY(VisibleAnywhere, Category = GunMesh)
		class UStaticMeshComponent* sniperGunComp;

	// �Ѿ����� ȿ�� ����
	UPROPERTY(EditAnywhere, Category = BulletEffect)
		class UParticleSystem* bulletEffectFactory;
	UPROPERTY(EditAnywhere, Category = BulletEffect)
		class UParticleSystem* bulletEffectFactory2;

	// �Ϲ� ���� ũ�ν����UI ����
	UPROPERTY(EditDefaultsOnly, Category = SniperUI)
		TSubclassOf<class UUserWidget> crosshairUIFactory;
	// ũ�ν���� �ν��Ͻ�
	class UUserWidget* _crosshairUI;

	// ������ ��� ������ ����
	bool bUsingGrenadeGun = true;
	// ��ź������ ����
	void ChangeToGrenadeGun();
	// �������۰����� ����
	void ChangeToSniperGun();

	// �Ѿ� �߻� ó�� �Լ�
	void InputFire();
	FTimerHandle timer;
	UPROPERTY()
		bool isFiring;
	void StartFire();
	void StopFire();
	void Fire();

	// �޸��� �̺�Ʈ ó�� �Լ�
	void InputRun();

	// ī�޶� ����ũ �������Ʈ�� ������ ����
	UPROPERTY(EditDefaultsOnly, Category = CameraMotion)
		TSubclassOf<class UCameraShakeBase> cameraShake;

	// �Ѿ� �߻� ����
	UPROPERTY(EditAnywhere, Category = Sound)
		class USoundBase* bulletSound;

	// ���� ü��
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Health)
		int32 hp;
	// �ʱ� HP��
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Health)
		int32 initialHp = 10;

	// �ǰ� ������ �� ó��
	UFUNCTION(BlueprintCallable, Category = Health)
		void OnHitEvent();

	// ���� ������ �� ȣ��� �Լ�
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Health)
		void OnGameOver();

	// �� �ٲ� �� ȣ��Ǵ� �̺�Ʈ �Լ�
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Health)
		void OnUsingGrenade(bool isGrenade);
};
