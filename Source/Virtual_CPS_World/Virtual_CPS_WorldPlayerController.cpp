// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Virtual_CPS_World.h"
#include "Virtual_CPS_WorldPlayerController.h"
#include "AI/Navigation/NavigationSystem.h"
#include "Sensor.h"
#include "DrawDebugHelpers.h"

AVirtual_CPS_WorldPlayerController::AVirtual_CPS_WorldPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
	UBlueprint* blueprint = Cast<UBlueprint>(StaticLoadObject(UObject::StaticClass(), NULL, TEXT("Blueprint'/Game/SensorNode.SensorNode'")));
	sensorBlueprint = (UClass*)(blueprint->GeneratedClass);
	if (sensorBlueprint != NULL) {
		UE_LOG(LogNet, Log, TEXT("BName: %s"), *(sensorBlueprint->GetClass()->GetName()));
	} else {
		UE_LOG(LogNet, Log, TEXT("I got nothing"));
	}
}

void AVirtual_CPS_WorldPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// keep updating the destination every tick while desired
	if (bMoveToMouseCursor)
	{
		MoveToMouseCursor();
	}
	if (bSelectItem) {
		selectItem();

		bSelectItem = false;
	}
}

void AVirtual_CPS_WorldPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &AVirtual_CPS_WorldPlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &AVirtual_CPS_WorldPlayerController::OnSetDestinationReleased);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AVirtual_CPS_WorldPlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AVirtual_CPS_WorldPlayerController::MoveToTouchLocation);

	InputComponent->BindAction("SelectItem", IE_Pressed, this, &AVirtual_CPS_WorldPlayerController::OnSelectItemPressed);
}

void AVirtual_CPS_WorldPlayerController::MoveToMouseCursor()
{
	// Trace to see what is under the mouse cursor
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	if (Hit.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(Hit.ImpactPoint);
	}
}

void AVirtual_CPS_WorldPlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;
	GetHitResultAtScreenPosition(ScreenSpaceLocation, CurrentClickTraceChannel, true, HitResult);
	if (HitResult.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(HitResult.ImpactPoint);
	}
}

void AVirtual_CPS_WorldPlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const Pawn = GetPawn();
	if (Pawn)
	{
		UNavigationSystem* const NavSys = GetWorld()->GetNavigationSystem();
		float const Distance = FVector::Dist(DestLocation, Pawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if (NavSys && (Distance > 120.0f))
		{
			NavSys->SimpleMoveToLocation(this, DestLocation);
		}
	}
}

void AVirtual_CPS_WorldPlayerController::OnSetDestinationPressed()
{
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void AVirtual_CPS_WorldPlayerController::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}

void AVirtual_CPS_WorldPlayerController::OnSelectItemPressed()
{
	bSelectItem = true;

}

void AVirtual_CPS_WorldPlayerController::selectItem() {
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	AActor *item = Hit.GetActor();
	if (item != NULL) {
		if (item->GetClass()->GetName().Compare(sensorBlueprint->GetName()) == 0){
			UE_LOG(LogNet, Log, TEXT("Clicked %s"), *(item->GetActorLabel()));
			ASensor *sensorActor = (ASensor *)item->GetAttachParentActor();
			DrawDebugCircle(GetWorld(), item->GetActorLocation(), 150.0, 360, FColor(0, 255, 0), false, 0.3, 0, 5, FVector(0.f,1.f,0.f), FVector(0.f,0.f,1.f), false);
		}
	}
}

