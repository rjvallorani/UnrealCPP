// Fill out your copyright notice in the Description page of Project Settings.


#include "explodeBarrel.h"

#include "NavigationSystemTypes.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AexplodeBarrel::AexplodeBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	RootComponent = BarrelMesh;

	FireParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BarrelMesh"));
	FireParticleSystemComponent->SetupAttachment(RootComponent);
	FireParticleSystemComponent->bAutoActivate = false;

	RadialForce = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForce"));

	RadialForce->Radius = ExplosionRadius;
	RadialForce->bImpulseVelChange = true;
	RadialForce->bAutoActivate = false;
	RadialForce->bIgnoreOwningActor = false;

	OnTakeAnyDamage.AddDynamic(this, &AexplodeBarrel::OnDamageTaken);

	HitPoints = 10.0f;
	ExplosionDamage = 5.0f;
	ExplosionRadius = 200.0f;
	

}

// Called when the game starts or when spawned
void AexplodeBarrel::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AexplodeBarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AexplodeBarrel::OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCause)
{
	if(Damage > 0.f && HitPoints > 0.f)
	{
		HitPoints-= Damage;
		UE_LOG(LogTemp, Warning, TEXT("Damage taken, Hitpoints: %f"), HitPoints); //log the remaining hit points

		//Start the fire particle system
		if (!FireParticleSystemComponent->IsActive())
		{
			FireParticleSystemComponent->Activate();
		}

		//Start a countdown if not started
		if(!GetWorld()->GetTimerManager().IsTimerActive(CountdownTimerHandle))
		{
			UE_LOG(LogTemp, Warning, TEXT("Setting countdown timer")); //Log if the timer is being set
			GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, this, &AexplodeBarrel::Countdown, 1.0f, true);
		}
	}

	//If hitpoints reach 0, the barrel explodes
	if (HitPoints<= 0.f)
	{
		//Stop fire particle system
		if (FireParticleSystemComponent!= nullptr)
		{
			FireParticleSystemComponent->Deactivate();
		}

		//Cache locaton for Spawning Events
		FVector BarrelLocation = GetActorLocation();

		Destroy();

		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, BarrelLocation, ExplosionRadius, nullptr, TArray<AActor*>(), this);

		if (RadialForce != nullptr)
		{
			RadialForce->FireImpulse();
		}

		//Play explosion sound and particle system
		UGameplayStatics::InternalSpawnEmitterAtLocation(GetWorld(), ExplosionParticle, BarrelLocation);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, BarrelLocation);
	}
}

void AexplodeBarrel::Countdown()
{
	HitPoints--;
	UE_LOG(LogTemp, Warning, TEXT("Countdown Called, Hitponts: %f"), HitPoints); //log when the countdown function is called and the remaining hitpoints

	//if hit points reach 0, the barrel explodes
	if (HitPoints <= 0.f)
	{
		OnDamageTaken(this, 0.0f, nullptr, nullptr, nullptr);
	}
}
