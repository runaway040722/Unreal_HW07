#include "MyPawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AMyPawn::AMyPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	SetRootComponent(CapsuleComp);
	CapsuleComp->SetSimulatePhysics(false);

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(RootComponent);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 400.0f;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);
}

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckGround();

	if (!bIsGrounded)
	{
		VerticalVelocity += Gravity * DeltaTime;
	}
	else if (VerticalVelocity < 0.f)
	{
		VerticalVelocity = 0.f;
	}

	FVector GravityOffset = FVector(0.f, 0.f, VerticalVelocity * DeltaTime);
	AddActorLocalOffset(GravityOffset, true);
}

void AMyPawn::CheckGround()
{
	FHitResult HitResult;
	FVector Start = GetActorLocation();
	FVector End = Start + (FVector::DownVector * 92.f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bIsGrounded = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
}

void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIP = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIP->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyPawn::Move);
		EIP->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyPawn::Look);
	}
}

void AMyPawn::Move(const FInputActionValue& Value)
{
	FVector2D MoveVec = Value.Get<FVector2D>();
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	if (Controller)
	{
		FVector Forward = GetActorForwardVector();
		FVector Right = GetActorRightVector();

		FVector Direction = (Forward * MoveVec.Y) + (Right * MoveVec.X);

		AddActorWorldOffset(Direction * MoveSpeed * DeltaTime, true);
	}
}

void AMyPawn::Look(const FInputActionValue& Value)
{
	FVector2D LookVec = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookVec.X);

		AddControllerPitchInput(LookVec.Y);
	}
}