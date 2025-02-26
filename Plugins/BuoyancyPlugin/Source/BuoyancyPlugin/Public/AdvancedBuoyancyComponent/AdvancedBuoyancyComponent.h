// For copyright see LICENSE in EnvironmentProject root dir, or:
//https://github.com/UE4-OceanProject/OceanProject/blob/Master-Environment-Project/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "OceanPlugin/Public/OceanManager.h"
#include "AdvancedBuoyancyComponent.generated.h"

USTRUCT(BlueprintType)
struct FBuoyancyVertex
{
	GENERATED_USTRUCT_BODY()

	FVector Position = FVector::ZeroVector;
	float Depth = 0.f;
};

USTRUCT(BlueprintType)
struct FForceTriangle
{
	GENERATED_USTRUCT_BODY()

	FBuoyancyVertex A; // A is the high or low vertex
	FBuoyancyVertex B; // B and C are the horizontal edge vertices
	FBuoyancyVertex C; // B and C are the horizontal edge vertices

	float TriArea;

	FBuoyancyVertex Center;
	FVector ForceCenter;
	FVector Normal;
	
	bool PointUp;
	bool CanSetForce = false;
	
	FVector Force;

	// Constructor
	FForceTriangle() {};
	FForceTriangle(FBuoyancyVertex AA, FBuoyancyVertex BB, FBuoyancyVertex CC, FVector BaseTriangleNormal, bool HorizontalUp)
	{
		A = AA;
		B = BB;
		C = CC;
		Normal = BaseTriangleNormal;
				
		FVector SideAB = B.Position - A.Position;
		FVector SideAC = C.Position - A.Position;
		float LengthMult = SideAB.Size() * SideAC.Size();
		TriArea = LengthMult * .5 * FGenericPlatformMath::Sin(FGenericPlatformMath::Acos(FVector::DotProduct(SideAB.GetSafeNormal(), SideAC.GetSafeNormal())));  // cm^2
		
		ForceCenter = FVector::ZeroVector;
		if (HorizontalUp) {
			FVector MedianPoint = (B.Position + C.Position) / 2;
			FVector MedianLine = A.Position - MedianPoint;
			float height = FMath::Abs(A.Position.Z - MedianPoint.Z);
			ForceCenter = MedianPoint + MedianLine * (2 * FMath::Abs(MedianPoint.Z) + height) / (6 * FMath::Abs(MedianPoint.Z) + 2 * height);
		}
		else
		{
			FVector MedianPoint = (B.Position + C.Position) / 2;
			FVector MedianLine = MedianPoint - A.Position;
			float height = FMath::Abs(A.Position.Z - MedianPoint.Z);
			ForceCenter = A.Position + MedianLine * (4 * FMath::Abs(A.Depth) + 3 * height) / (6 * FMath::Abs(A.Depth) + 4 * height);
		}
	};

	void SetForce()
	{
		if (CanSetForce) {
			Force = -Normal * TriArea * FMath::Abs(Center.Depth);
		}
		else
		{
			Force = FVector::ZeroVector;
		}
	};

};

UCLASS( ClassGroup=(Physics), meta=(BlueprintSpawnableComponent) )
class BUOYANCYPLUGIN_API UAdvancedBuoyancyComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Constructor
	UAdvancedBuoyancyComponent();

	virtual void InitializeComponent() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	
	// use drag when using advanced buoyancy (most likely will always be yes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
		bool bUseDrag = true;
	// will draw force arrows and buoyancy traingles/points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Options")
		bool bDebugOn = false;

	// World information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Data")
		AOceanManager* TheOcean = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Data")
		float WaterDensity = 1025.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Data")
		float Gravity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
		float MeshDensity = 800.f;
	UFUNCTION(BlueprintCallable, Category = "World Data")
		void GetOcean();

	UFUNCTION(BlueprintCallable, Category = "Debug")
		void DrawDebugStuff(FForceTriangle TriForce, FColor DebugColor);
	UFUNCTION(BlueprintCallable, Category = "Mesh")
		void SetMeshDensity(float NewDensity, float NewWaterDensity = 1025.f);

private:

	UWorld* World;
	float CorrectedMeshDensity;
	float CorrectedWaterDensity;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Advanced buoyancy using a static mesh triangles
	//////////////////////////////////////////////////////////////////////////////////////////////////
public:
		
	// The mesh the forces are being applied too, can be different from the mesh used for buoyancy.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Mesh Data")
		UStaticMeshComponent* BuoyantMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Mesh Data")
		FTransform MeshTransform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Mesh Data")
		float FalseVolume = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Coefficients")
		float BuoyancyReductionCoefficient = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Coefficients")
		float BuoyancyPitchReductionCoefficient = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Coefficients")
		float DensityCorrection = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Coefficients")
		float DensityCorrectionModifier = .1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Coefficients")
		float SubmergedVolume = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Coefficients")
		float ImpactCoefficient = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Coefficients")
		FVector DragCoefficient = FVector(15.f, 15.f, 1.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Coefficients")
		FVector SuctionCoefficient = FVector(15.f, 15.f, 1.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Coefficients")
		float ViscousDragCoefficient = .09f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Coefficients")
		float MaxSlamAcceleration = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Optimization")
		TArray<FVector> AdvancedGridHeight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Triangles")
		TArray<FForceTriangle> SubmergedTris;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Triangles")
		TArray<float> TriSizes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Buoyancy|Triangles")
		TArray<float> TriSubmergedArea;

	UFUNCTION(BlueprintCallable, Category = "Advanced Buoyancy|Forces")
		void ApplySlamForce(FVector SlamForce, FVector TriCenter);
	UFUNCTION(BlueprintCallable, Category = "Advanced Buoyancy|Triangles")
		TArray<FForceTriangle> SplitTriangle(FBuoyancyVertex H, FBuoyancyVertex M, FBuoyancyVertex L, FVector InArrow);
	UFUNCTION(BlueprintCallable, Category = "Advanced Buoyancy|Triangles")
		void ApplyForce(FForceTriangle TriForce);
	UFUNCTION(BlueprintCallable, Category = "Advanced Buoyancy|Triangles")
		float GetOceanDepthFromGrid(FVector Position, bool bJustGetHeightAtLocation = false);
	UFUNCTION(BlueprintCallable, Category = "Advanced Buoyancy|Triangles")
		float TriangleArea(FVector A, FVector B, FVector C);

private:

	void AdvancedBuoyancy();

	float ForceC;      // result of multiplication used elsewhere that will not change
	FVector MinBound;  // mesh bounds
	FVector MaxBound;  // mesh bounds
	TArray< TArray<FVector> > Triangles;  // an array of the triangles
	void PopulateTrianglesFromStaticMesh();
	
};
