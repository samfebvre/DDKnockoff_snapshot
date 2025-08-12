#pragma once

class UTestWorldEngineSubsystem;

class DDKNOCKOFFTESTS_API FTestWorldHelper {
    // Subsystem that manages the created Worlds
    UTestWorldEngineSubsystem* Subsystem;
    UWorld* World;
    // If NOT shared, then we need to cleanup it when we are destroyed
    bool IsSharedWorld;
    decltype(GFrameCounter) OldGFrameCounter;

public:
    explicit FTestWorldHelper();

    explicit FTestWorldHelper(UTestWorldEngineSubsystem* Subsystem,
                              UWorld* World,
                              bool IsSharedWorld);

    // This class is like std::unique_ptr for UWorld instances
    FTestWorldHelper(const FTestWorldHelper&) = delete;
    FTestWorldHelper& operator=(const FTestWorldHelper&) = delete;

    FTestWorldHelper(FTestWorldHelper&& Other) noexcept;
    FTestWorldHelper& operator=(FTestWorldHelper&& Other) noexcept;

    ~FTestWorldHelper();
    void Cleanup();

    UWorld* GetWorld() const;

    // Smallest DeltaTime with an exact representation.
    void Tick(float DeltaTime = 0.001953125) const;
};
