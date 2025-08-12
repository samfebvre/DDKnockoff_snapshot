#pragma once

#include "CoreMinimal.h"
#include "ManagerHandlerSubsystem.generated.h"

class UManagerBase;

/**
 * Central subsystem for managing and providing access to game managers.
 * Handles lifecycle, registration, and tick distribution for all manager instances.
 * Provides static and instance-based access patterns for efficient manager retrieval.
 */
UCLASS()
class DDKNOCKOFF_API UManagerHandlerSubsystem : public UTickableWorldSubsystem {
    GENERATED_BODY()

public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Manager access

    /**
     * Static convenience method for manager access from any world context.
     * @param World - World context to get subsystem from
     * @return Manager instance of type T or null if not found
     */
    template <typename T>
    static T* GetManager(UWorld* World) {
        return World->GetSubsystem<UManagerHandlerSubsystem>()->GetManager<T>();
    }

    /**
     * Get manager instance by template type.
     * @return Manager instance of type T or null if not found
     */
    template <typename T>
    T* GetManager() {
        auto FoundValue = Managers.Find(T::StaticClass());
        if (FoundValue) { return Cast<T>(*FoundValue); }
        return nullptr;
    }

    // Manager lifecycle

    /**
     * Initialize managers from provided class list.
     * @param ManagerClasses - Array of manager classes to instantiate and register
     */
    void SetManagers(const TArray<UClass*>& ManagerClasses);

    /**
     * Clean up and destroy all managed instances.
     */
    void ClearManagers();

    /**
     * Reinitialize all managers with current configuration.
     */
    void RefreshManagers();

    // UTickableWorldSubsystem Interface
    virtual bool IsTickable() const override { return true; }
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

private:
    // Manager storage

    UPROPERTY(Transient)
    TMap<UClass*, UManagerBase*> Managers;
};
