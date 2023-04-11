// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include "Bullet.h"
#include <Kismet/GameplayStatics.h>
#include <Blueprint/UserWidget.h>
#include <GameFramework/CharacterMovementComponent.h>
#include "PlayerAnim.h"
#include "EnemyFSM.h"
#include "Project.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
ATPSPlayer::ATPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh(TEXT("SkeletalMesh'/Game/03_Meshes/Belica_PolarStrike.Belica_PolarStrike'"));

	if (TempMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(TempMesh.Object);

		GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90), FRotator(0, -90, 0));
	}

	springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	springArmComp->SetupAttachment(RootComponent);
	springArmComp->SetRelativeLocation(FVector(0, 100, 90));
	springArmComp->TargetArmLength = 400;
	springArmComp->bUsePawnControlRotation = true;

	tpsCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("TpsCamComp"));
	tpsCamComp->SetupAttachment(springArmComp);
	tpsCamComp->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = true;

	// 2�� ����
	JumpMaxCount = 1;

	// 4. �� ���̷�Ż�޽� ������Ʈ ���
	gunMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMeshComp"));
	// 4-1. �θ� ������Ʈ�� Mesh ������Ʈ�� ����
	gunMeshComp->SetupAttachment(GetMesh());
	// 4-2. ���̷�Ż�޽� ������ �ε�
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempGunMesh(TEXT("SkeletalMesh'/Game/03_Meshes/SK_FPGun.SK_FPGun'"));
	// 4-3. �����ͷε尡 �����ߴٸ�
	if (TempGunMesh.Succeeded())
	{
		// 4-4. ���̷�Ż�޽� ������ �Ҵ�
		gunMeshComp->SetSkeletalMesh(TempGunMesh.Object);
		// 4-5. ��ġ �����ϱ�
		gunMeshComp->SetRelativeLocation(FVector(-14, 52, 120));
	}

	// 5. �������۰� ������Ʈ ���
	sniperGunComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT
	("SniperGunComp"));
	// 5-1. �θ� ������Ʈ�� Mesh ������Ʈ�� ����
	sniperGunComp->SetupAttachment(GetMesh());
	// 5-2. ����ƽ�޽� ������ �ε�
	ConstructorHelpers::FObjectFinder<UStaticMesh> TempSniperMesh(TEXT
	("StaticMesh'/Game/03_Meshes/sniper1.sniper1'"));
	// 5-3. �����ͷε尡 �����ߴٸ�
	if (TempSniperMesh.Succeeded())
	{
		// 5-4. ����ƽ�޽� ������ �Ҵ�
		sniperGunComp->SetStaticMesh(TempSniperMesh.Object);
		// 5-5. ��ġ �����ϱ�
		sniperGunComp->SetRelativeLocation(FVector(-22, 55, 120));
		// 5-6. ũ�� �����ϱ�
		sniperGunComp->SetRelativeScale3D(FVector(0.15f));
	}

	// �Ѿ� ���� ��������
	ConstructorHelpers::FObjectFinder<USoundBase> tempSound(TEXT
	("SoundWave'/Game/07_Asset/Audio/SniperSound.SniperSound'"));
	if (tempSound.Succeeded())
	{
		bulletSound = tempSound.Object;
	}
}

// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	// �ʱ� �ӵ��� �ȱ�� ����
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;

	// 1. �������� UI���� �ν��Ͻ� ����

	
	// 2. �Ϲ� ���� UIũ�ν���� �ν��Ͻ� ����
	_crosshairUI = CreateWidget(GetWorld(), crosshairUIFactory);
	// 3. �Ϲ� ���� UI���
	_crosshairUI->AddToViewport();

	// �⺻���� �������۰��� ����ϵ��� ����
	ChangeToSniperGun();
	
	hp = initialHp;
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Move();
}

// Called to bind functionality to input
void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ATPSPlayer::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ATPSPlayer::LookUp);

	// �¿� �Է� �̺�Ʈ ó�� �Լ� ���ε�
	PlayerInputComponent->BindAxis(TEXT("Horizontal"), this, &ATPSPlayer::InputHorizontal);
	// ���� �Է� �̺�Ʈ ó�� �Լ� ���ε�
	PlayerInputComponent->BindAxis(TEXT("Vertical"), this, &ATPSPlayer::InputVertical);

	// ���� �Է� �̺�Ʈ ó�� �Լ� ���ε�
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ATPSPlayer::InputJump);

	// �Ѿ� �߻� �̺�Ʈ ó�� �Լ� ���ε�
	//PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ATPSPlayer::InputFire);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ATPSPlayer::StartFire);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &ATPSPlayer::StopFire);

	// �� ��ü �̺�Ʈ ó�� �Լ� ���ε�
	PlayerInputComponent->BindAction(TEXT("GrenadeGun"), IE_Pressed, this, &ATPSPlayer::ChangeToGrenadeGun);
	PlayerInputComponent->BindAction(TEXT("SniperGun"), IE_Pressed, this, &ATPSPlayer::ChangeToSniperGun);

	// �޸��� �Է� �̺�Ʈ ó�� �Լ� ���ε�
	PlayerInputComponent->BindAction(TEXT("Run"), IE_Pressed, this, &ATPSPlayer::InputRun);
	PlayerInputComponent->BindAction(TEXT("Run"), IE_Released, this, &ATPSPlayer::InputRun);
}

void ATPSPlayer::InputRun()
{
	auto movement = GetCharacterMovement();

	// ���� �޸��� �����
	if (movement->MaxWalkSpeed > walkSpeed)
	{
		// �ȱ� �ӵ��� ��ȯ
		movement->MaxWalkSpeed = walkSpeed;
	}
	else
	{
		movement->MaxWalkSpeed = runSpeed;
	}
}

void ATPSPlayer::Move()
{
	// �÷��̾� �̵� ó��
	// ��ӿ
	// P(��� ��ġ) = PO(������ġ) + v(�ӵ�) x t(�ð�)
	direction = FTransform(GetControlRotation()).TransformVector(direction);
	/*FVector P0 = GetActorLocation();
	FVector vt = direction * walkSpeed * DeltaTime;
	FVector P = P0 + vt;
	SetActorLocation(P);*/
	AddMovementInput(direction);
	direction = FVector::ZeroVector;
}

//void ATPSPlayer::InputFire()
//{
//	// ��ź�� ��� ��
//	if (bUsingGrenadeGun)
//	{
//		//// �Ѿ� �߻� ó��
//		//FTransform firePosition = gunMeshComp->GetSocketTransform(TEXT("FirePosition"));
//		//GetWorld()->SpawnActor<ABullet>(bulletFactory, firePosition);
//		StartFire();
//		StopFire();
//	}
//	// �������۰� ��� ��
//	else
//	{
//		// LineTrace�� ���� ��ġ
//		FVector startPos = tpsCamComp->GetComponentLocation();
//		// LineTrace�� ���� ��ġ
//		FVector endPos = tpsCamComp->GetComponentLocation() + tpsCamComp->GetForwardVector() * 5000;
//		// LineTrace�� �浹 ������ ���� ����
//		FHitResult hitInfo;
//		// �浹 �ɼ� ���� ����
//		FCollisionQueryParams params;
//		// �ڱ� �ڽ�(�÷��̾�)�� �浹���� ����
//		params.AddIgnoredActor(this);
//		// Channel ���͸� �̿��� LineTrace �浹 ����(�浹 ����, ���� ��ġ, ���� ��ġ, ���� ä��, �浹 �ɼ�)
//		bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECC_Visibility, params);
//		// LineTrace�� �ε����� ��
//		if (bHit)
//		{
//			// �浹 ó�� -> �Ѿ� ���� ȿ�� ���
//			// �Ѿ� ���� ȿ�� Ʈ������
//			FTransform bulletTrans;
//			// �ε��� ��ġ �Ҵ�
//			bulletTrans.SetLocation(hitInfo.ImpactPoint);
//			// �Ѿ� ���� ȿ�� �ν��Ͻ� ����
//			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), bulletEffectFactory, bulletTrans);
//
//			auto hitComp = hitInfo.GetComponent();
//			// 1. ���� ������Ʈ�� ������ ����Ǿ� �ִٸ�
//			if (hitComp && hitComp->IsSimulatingPhysics())
//			{
//				// 2. �������� ���� ������ �ʿ�
//				FVector force = -hitInfo.ImpactNormal * hitComp->GetMass() * 500000;
//				// 3. �� �������� ���������� �ʹ�.
//				hitComp->AddForce(force);
//			}
//		}
//	}
//}

void ATPSPlayer::StartFire()
{
	isFiring = true;
	Fire();
}

void ATPSPlayer::StopFire()
{
	isFiring = false;
}

void ATPSPlayer::Fire()
{
	UGameplayStatics::PlaySound2D(GetWorld(), bulletSound);

	// ī�޶� ����ũ ���
	auto controller = GetWorld()->GetFirstPlayerController();
	controller->PlayerCameraManager->StartCameraShake(cameraShake);

	// ���� �ִϸ��̼� ���
	auto anim = Cast<UPlayerAnim>(GetMesh()->GetAnimInstance());
	anim->PlayAttackAnim();

	if (bUsingGrenadeGun)
	{
		/*if (isFiring)
		{
			FTransform firePosition = gunMeshComp->GetSocketTransform(TEXT("FirePosition"));
			GetWorld()->SpawnActor<ABullet>(bulletFactory, firePosition);
			GetWorld()->GetTimerManager().SetTimer(timer, this, &ATPSPlayer::Fire, .1f, false);
		}*/		

		if (isFiring)
		{
			// LineTrace�� ���� ��ġ
			FVector startPos = tpsCamComp->GetComponentLocation();
			// LineTrace�� ���� ��ġ
			FVector endPos = tpsCamComp->GetComponentLocation() + tpsCamComp->GetForwardVector() * 5000;
			// LineTrace�� �浹 ������ ���� ����
			FHitResult hitInfo;
			// �浹 �ɼ� ���� ����
			FCollisionQueryParams params;
			// �ڱ� �ڽ�(�÷��̾�)�� �浹���� ����
			params.AddIgnoredActor(this);
			// Channel ���͸� �̿��� LineTrace �浹 ����(�浹 ����, ���� ��ġ, ���� ��ġ, ���� ä��, �浹 �ɼ�)
			bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECC_Visibility, params);
			// LineTrace�� �ε����� ��
			if (bHit)
			{
				// �浹 ó�� -> �Ѿ� ���� ȿ�� ���
				// �Ѿ� ���� ȿ�� Ʈ������
				FTransform bulletTrans;
				// �ε��� ��ġ �Ҵ�
				bulletTrans.SetLocation(hitInfo.ImpactPoint);
				// �Ѿ� ���� ȿ�� �ν��Ͻ� ����
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), bulletEffectFactory2, bulletTrans);

				auto hitComp = hitInfo.GetComponent();
				// 1. ���� ������Ʈ�� ������ ����Ǿ� �ִٸ�
				if (hitComp && hitComp->IsSimulatingPhysics())
				{
					// 2. �������� ���� ������ �ʿ�
					FVector force = -hitInfo.ImpactNormal * hitComp->GetMass() * 300000;
					// 3. �� �������� ���������� �ʹ�.
					hitComp->AddForce(force);
				}
			}
			GetWorld()->GetTimerManager().SetTimer(timer, this, &ATPSPlayer::Fire, .1f, false);

			// �ε��� ����� ������ �Ǵ��ϱ�
			auto enemy = hitInfo.GetActor()->GetDefaultSubobjectByName(TEXT("FSM"));
			if (enemy)
			{
				auto enemyFSM = Cast<UEnemyFSM>(enemy);
				enemyFSM->OnDamageProcess();
			}
		}
	}

	else
	{
		// LineTrace�� ���� ��ġ
		FVector startPos = tpsCamComp->GetComponentLocation();
		// LineTrace�� ���� ��ġ
		FVector endPos = tpsCamComp->GetComponentLocation() + tpsCamComp->GetForwardVector() * 5000;
		// LineTrace�� �浹 ������ ���� ����
		FHitResult hitInfo;
		// �浹 �ɼ� ���� ����
		FCollisionQueryParams params;
		// �ڱ� �ڽ�(�÷��̾�)�� �浹���� ����
		params.AddIgnoredActor(this);
		// Channel ���͸� �̿��� LineTrace �浹 ����(�浹 ����, ���� ��ġ, ���� ��ġ, ���� ä��, �浹 �ɼ�)
		bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECC_Visibility, params);
		// LineTrace�� �ε����� ��
		if (bHit)
		{
			// �浹 ó�� -> �Ѿ� ���� ȿ�� ���
			// �Ѿ� ���� ȿ�� Ʈ������
			FTransform bulletTrans;
			// �ε��� ��ġ �Ҵ�
			bulletTrans.SetLocation(hitInfo.ImpactPoint);
			// �Ѿ� ���� ȿ�� �ν��Ͻ� ����
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), bulletEffectFactory, bulletTrans);

			auto hitComp = hitInfo.GetComponent();
			// 1. ���� ������Ʈ�� ������ ����Ǿ� �ִٸ�
			if (hitComp && hitComp->IsSimulatingPhysics())
			{
				// 2. �������� ���� ������ �ʿ�
				FVector force = -hitInfo.ImpactNormal * hitComp->GetMass() * 500000;
				// 3. �� �������� ���������� �ʹ�.
				hitComp->AddForce(force);
			}

			// �ε��� ����� ������ �Ǵ��ϱ�
			auto enemy = hitInfo.GetActor()->GetDefaultSubobjectByName(TEXT("FSM"));
			if (enemy)
			{
				auto enemyFSM = Cast<UEnemyFSM>(enemy);
				enemyFSM->OnDamageProcess();
			}
		}
	}
}

void ATPSPlayer::InputJump()
{
	Jump();
}

// �¿� �Է� �̺�Ʈ ó�� �Լ�
void ATPSPlayer::InputHorizontal(float value)
{
	direction.Y = value;
}
// ���� �Է� �̺�Ʈ ó�� �Լ�
void ATPSPlayer::InputVertical(float value)
{
	direction.X = value;
}

void ATPSPlayer::Turn(float value)
{
	AddControllerYawInput(value);
}

void ATPSPlayer::LookUp(float value)
{
	AddControllerPitchInput(value);
}

void ATPSPlayer::ChangeToGrenadeGun()
{
	// ������ ��� ������ üũ
	bUsingGrenadeGun = true;
	sniperGunComp->SetVisibility(false);
	gunMeshComp->SetVisibility(true);

	// �������� ������� ���� ����
	OnUsingGrenade(bUsingGrenadeGun);
}

void ATPSPlayer::ChangeToSniperGun()
{
	bUsingGrenadeGun = false;
	sniperGunComp->SetVisibility(true);
	gunMeshComp->SetVisibility(false);

	// �������� ������� ���� ����
	OnUsingGrenade(bUsingGrenadeGun);
}

void ATPSPlayer::OnHitEvent()
{
	PRINT_LOG(TEXT("Damage!!!!!"));
	hp--;
	if (hp <= 0)
	{
		PRINT_LOG(TEXT("Player is dead!"));
		OnGameOver();
	}
}

void ATPSPlayer::OnGameOver_Implementation()
{
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}