#include "LevelLogic/WaveManagerSettings.h"
#include "LevelLogic/LevelData.h"
#include "Core/ConfigurationValidatable.h"

void UWaveManagerSettings::ValidateConfiguration() const {
    // Validate wave countdown duration
    ensureAlwaysMsgf(WaveCountdownDuration >= 0.0f,
                     TEXT(
                         "WaveManagerSettings: WaveCountdownDuration cannot be negative (current: %f)"
                     ),
                     WaveCountdownDuration);

    // Validate time between spawns
    ensureAlwaysMsgf(TimeBetweenSpawns >= 0.0f,
                     TEXT("WaveManagerSettings: TimeBetweenSpawns cannot be negative (current: %f)"
                     ),
                     TimeBetweenSpawns);

    // Validate level data
    ensureAlwaysMsgf(LevelData != nullptr,
                     TEXT(
                         "WaveManagerSettings: LevelData is not set! Wave manager will not function correctly"
                     ));

    // Validate level data contents if it implements IConfigurationValidatable
    if (LevelData && Cast<IConfigurationValidatable>(LevelData)) {
        Cast<IConfigurationValidatable>(LevelData)->ValidateConfiguration();
    }
}
