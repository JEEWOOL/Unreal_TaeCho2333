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

	// 2단 점프
	JumpMaxCount = 1;

	// 4. 총 스켈레탈메시 컴포넌트 등록
	gunMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMeshComp"));
	// 4-1. 부모 컴포넌트를 Mesh 컴포넌트로 설정
	gunMeshComp->SetupAttachment(GetMesh());
	// 4-2. 스켈레탈메시 데이터 로드
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempGunMesh(TEXT("SkeletalMesh'/Game/03_Meshes/SK_FPGun.SK_FPGun'"));
	// 4-3. 데이터로드가 성공했다면
	if (TempGunMesh.Succeeded())
	{
		// 4-4. 스켈레탈메시 데이터 할당
		gunMeshComp->SetSkeletalMesh(TempGunMesh.Object);
		// 4-5. 위치 조정하기
		gunMeshComp->SetRelativeLocation(FVector(-14, 52, 120));
	}

	// 5. 스나이퍼건 컴포넌트 등록
	sniperGunComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT
	("SniperGunComp"));
	// 5-1. 부모 컴포넌트를 Mesh 컴포넌트로 설정
	sniperGunComp->SetupAttachment(GetMesh());
	// 5-2. 스테틱메시 데이터 로드
	ConstructorHelpers::FObjectFinder<UStaticMesh> TempSniperMesh(TEXT
	("StaticMesh'/Game/03_Meshes/sniper1.sniper1'"));
	// 5-3. 데이터로드가 성공했다면
	if (TempSniperMesh.Succeeded())
	{
		// 5-4. 스테틱메시 데이터 할당
		sniperGunComp->SetStaticMesh(TempSniperMesh.Object);
		// 5-5. 위치 조정하기
		sniperGunComp->SetRelativeLocation(FVector(-22, 55, 120));
		// 5-6. 크기 조정하기
		sniperGunComp->SetRelativeScale3D(FVector(0.15f));
	}

	// 총알 사운드 가져오기
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

	// 초기 속도를 걷기로 설정
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;

	// 1. 스나이퍼 UI위젯 인스턴스 생성

	
	// 2. 일반 조준 UI크로스헤어 인스턴스 생성
	_crosshairUI = CreateWidget(GetWorld(), crosshairUIFactory);
	// 3. 일반 조준 UI등록
	_crosshairUI->AddToViewport();

	// 기본으로 스나이퍼건을 사용하도록 설정
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

	// 좌우 입력 이벤트 처리 함수 바인딩
	PlayerInputComponent->BindAxis(TEXT("Horizontal"), this, &ATPSPlayer::InputHorizontal);
	// 상하 입력 이벤트 처리 함수 바인딩
	PlayerInputComponent->BindAxis(TEXT("Vertical"), this, &ATPSPlayer::InputVertical);

	// 점프 입력 이벤트 처리 함수 바인딩
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ATPSPlayer::InputJump);

	// 총알 발사 이벤트 처리 함수 바인딩
	//PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ATPSPlayer::InputFire);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ATPSPlayer::StartFire);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &ATPSPlayer::StopFire);

	// 총 교체 이벤트 처리 함수 바인딩
	PlayerInputComponent->BindAction(TEXT("GrenadeGun"), IE_Pressed, this, &ATPSPlayer::ChangeToGrenadeGun);
	PlayerInputComponent->BindAction(TEXT("SniperGun"), IE_Pressed, this, &ATPSPlayer::ChangeToSniperGun);

	// 달리기 입력 이벤트 처리 함수 바인딩
	PlayerInputComponent->BindAction(TEXT("Run"), IE_Pressed, this, &ATPSPlayer::InputRun);
	PlayerInputComponent->BindAction(TEXT("Run"), IE_Released, this, &ATPSPlayer::InputRun);
}

void ATPSPlayer::InputRun()
{
	auto movement = GetCharacterMovement();

	// 현재 달리기 모드라면
	if (movement->MaxWalkSpeed > walkSpeed)
	{
		// 걷기 속도로 전환
		movement->MaxWalkSpeed = walkSpeed;
	}
	else
	{
		movement->MaxWalkSpeed = runSpeed;
	}
}

void ATPSPlayer::Move()
{
	// 플레이어 이동 처리
	// 등속운동
	// P(결과 위치) = PO(현재위치) + v(속도) x t(시간)
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
//	// 유탄총 사용 시
//	if (bUsingGrenadeGun)
//	{
//		//// 총알 발사 처리
//		//FTransform firePosition = gunMeshComp->GetSocketTransform(TEXT("FirePosition"));
//		//GetWorld()->SpawnActor<ABullet>(bulletFactory, firePosition);
//		StartFire();
//		StopFire();
//	}
//	// 스나이퍼건 사용 시
//	else
//	{
//		// LineTrace의 시작 위치
//		FVector startPos = tpsCamComp->GetComponentLocation();
//		// LineTrace의 종료 위치
//		FVector endPos = tpsCamComp->GetComponentLocation() + tpsCamComp->GetForwardVector() * 5000;
//		// LineTrace의 충돌 정보를 담을 변수
//		FHitResult hitInfo;
//		// 충돌 옵션 설정 변수
//		FCollisionQueryParams params;
//		// 자기 자신(플레이어)는 충돌에서 제외
//		params.AddIgnoredActor(this);
//		// Channel 필터를 이용한 LineTrace 충돌 검출(충돌 정보, 시작 위치, 종료 위치, 검출 채널, 충돌 옵션)
//		bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECC_Visibility, params);
//		// LineTrace가 부딪혔을 때
//		if (bHit)
//		{
//			// 충돌 처리 -> 총알 파편 효과 재생
//			// 총알 파편 효과 트랜스폼
//			FTransform bulletTrans;
//			// 부딪힌 위치 할당
//			bulletTrans.SetLocation(hitInfo.ImpactPoint);
//			// 총알 파편 효과 인스턴스 생성
//			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), bulletEffectFactory, bulletTrans);
//
//			auto hitComp = hitInfo.GetComponent();
//			// 1. 만약 컴포넌트레 물리가 적용되어 있다면
//			if (hitComp && hitComp->IsSimulatingPhysics())
//			{
//				// 2. 날려버릴 힘과 방향이 필요
//				FVector force = -hitInfo.ImpactNormal * hitComp->GetMass() * 500000;
//				// 3. 그 방향으로 날려버리고 싶다.
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

	// 카메라 셰이크 재생
	auto controller = GetWorld()->GetFirstPlayerController();
	controller->PlayerCameraManager->StartCameraShake(cameraShake);

	// 공격 애니메이션 재생
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
			// LineTrace의 시작 위치
			FVector startPos = tpsCamComp->GetComponentLocation();
			// LineTrace의 종료 위치
			FVector endPos = tpsCamComp->GetComponentLocation() + tpsCamComp->GetForwardVector() * 5000;
			// LineTrace의 충돌 정보를 담을 변수
			FHitResult hitInfo;
			// 충돌 옵션 설정 변수
			FCollisionQueryParams params;
			// 자기 자신(플레이어)는 충돌에서 제외
			params.AddIgnoredActor(this);
			// Channel 필터를 이용한 LineTrace 충돌 검출(충돌 정보, 시작 위치, 종료 위치, 검출 채널, 충돌 옵션)
			bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECC_Visibility, params);
			// LineTrace가 부딪혔을 때
			if (bHit)
			{
				// 충돌 처리 -> 총알 파편 효과 재생
				// 총알 파편 효과 트랜스폼
				FTransform bulletTrans;
				// 부딪힌 위치 할당
				bulletTrans.SetLocation(hitInfo.ImpactPoint);
				// 총알 파편 효과 인스턴스 생성
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), bulletEffectFactory2, bulletTrans);

				auto hitComp = hitInfo.GetComponent();
				// 1. 만약 컴포넌트레 물리가 적용되어 있다면
				if (hitComp && hitComp->IsSimulatingPhysics())
				{
					// 2. 날려버릴 힘과 방향이 필요
					FVector force = -hitInfo.ImpactNormal * hitComp->GetMass() * 300000;
					// 3. 그 방향으로 날려버리고 싶다.
					hitComp->AddForce(force);
				}
			}
			GetWorld()->GetTimerManager().SetTimer(timer, this, &ATPSPlayer::Fire, .1f, false);

			// 부딪힌 대상이 적인지 판단하기
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
		// LineTrace의 시작 위치
		FVector startPos = tpsCamComp->GetComponentLocation();
		// LineTrace의 종료 위치
		FVector endPos = tpsCamComp->GetComponentLocation() + tpsCamComp->GetForwardVector() * 5000;
		// LineTrace의 충돌 정보를 담을 변수
		FHitResult hitInfo;
		// 충돌 옵션 설정 변수
		FCollisionQueryParams params;
		// 자기 자신(플레이어)는 충돌에서 제외
		params.AddIgnoredActor(this);
		// Channel 필터를 이용한 LineTrace 충돌 검출(충돌 정보, 시작 위치, 종료 위치, 검출 채널, 충돌 옵션)
		bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECC_Visibility, params);
		// LineTrace가 부딪혔을 때
		if (bHit)
		{
			// 충돌 처리 -> 총알 파편 효과 재생
			// 총알 파편 효과 트랜스폼
			FTransform bulletTrans;
			// 부딪힌 위치 할당
			bulletTrans.SetLocation(hitInfo.ImpactPoint);
			// 총알 파편 효과 인스턴스 생성
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), bulletEffectFactory, bulletTrans);

			auto hitComp = hitInfo.GetComponent();
			// 1. 만약 컴포넌트에 물리가 적용되어 있다면
			if (hitComp && hitComp->IsSimulatingPhysics())
			{
				// 2. 날려버릴 힘과 방향이 필요
				FVector force = -hitInfo.ImpactNormal * hitComp->GetMass() * 500000;
				// 3. 그 방향으로 날려버리고 싶다.
				hitComp->AddForce(force);
			}

			// 부딪힌 대상이 적인지 판단하기
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

// 좌우 입력 이벤트 처리 함수
void ATPSPlayer::InputHorizontal(float value)
{
	direction.Y = value;
}
// 상하 입력 이벤트 처리 함수
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
	// 라이플 사용 중으로 체크
	bUsingGrenadeGun = true;
	sniperGunComp->SetVisibility(false);
	gunMeshComp->SetVisibility(true);

	// 라이플을 사용할지 여부 전달
	OnUsingGrenade(bUsingGrenadeGun);
}

void ATPSPlayer::ChangeToSniperGun()
{
	bUsingGrenadeGun = false;
	sniperGunComp->SetVisibility(true);
	gunMeshComp->SetVisibility(false);

	// 라이플을 사용할지 여부 전달
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