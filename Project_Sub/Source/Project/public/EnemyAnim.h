// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyFSM.h"
#include "EnemyAnim.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_API UEnemyAnim : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// ���¸ӽ� ��� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = FSM)
		EEnemyState animState;
	// ���� ���� ������� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = FSM)
		bool bAttackPlay = false;
	//���� �ִϸ��̼� ������ �̺�Ʈ �Լ�
	UFUNCTION(BlueprintCallable, Category = FSMEvent)
		void OnEndAttackAnimation();
	// �ǰ� �ִϸ��̼� ��� �Լ�
	UFUNCTION(BlueprintImplementableEvent, Category = FSMEvent)
		void PlayDamageAnim(FName sectionName);

	// ���� ���� �ִϸ��̼� ���� ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = FSM)
		bool bDieDone = false;
};
