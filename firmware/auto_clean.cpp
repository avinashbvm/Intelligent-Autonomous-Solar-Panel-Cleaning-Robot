#include "auto_clean.h"
#include "config.h"

AutoCleanManager::AutoCleanManager(VoltageSensor &sensor, Cleaner &cleaner)
    : _sensor(sensor), _cleaner(cleaner) {
}

void AutoCleanManager::begin() {
    _dropThresholdPercent = DEFAULT_DROP_THRESHOLD_PERCENT;
}

void AutoCleanManager::setEnabled(bool enabled) {
    _enabled = enabled;

    if (!enabled) {
        _lowVoltageStartTime = 0;
    }
}

bool AutoCleanManager::isEnabled() const {
    return _enabled;
}

void AutoCleanManager::setBaseline(float volts) {
    if (volts >= MIN_VALID_PANEL_VOLTAGE) {
        _baselineVoltage = volts;

        // A manually supplied new baseline means we can resume monitoring.
        _state = MONITORING;
        _lowVoltageStartTime = 0;
    }
}

void AutoCleanManager::setBaselineFromCurrentVoltage() {
    setBaseline(_sensor.getVoltage());
}

float AutoCleanManager::getBaseline() const {
    return _baselineVoltage;
}

void AutoCleanManager::setDropThresholdPercent(float percent) {
    if (percent >= 1.0f && percent <= 90.0f) {
        _dropThresholdPercent = percent;
    }
}

float AutoCleanManager::getDropThresholdPercent() const {
    return _dropThresholdPercent;
}

float AutoCleanManager::calculateDropPercent(float currentVoltage) const {
    if (_baselineVoltage < MIN_VALID_PANEL_VOLTAGE) {
        return 0.0f;
    }

    float drop =
        ((_baselineVoltage - currentVoltage) / _baselineVoltage) * 100.0f;

    if (drop < 0.0f) {
        drop = 0.0f;
    }

    return drop;
}

float AutoCleanManager::getDropPercent() const {
    return calculateDropPercent(_sensor.getVoltage());
}

bool AutoCleanManager::cooldownFinished() const {
    return _lastAutoCleanTime == 0 ||
           (millis() - _lastAutoCleanTime >= AUTO_CLEAN_COOLDOWN_MS);
}

void AutoCleanManager::update() {
    const float currentVoltage = _sensor.getVoltage();

    if (!_enabled || currentVoltage < MIN_VALID_PANEL_VOLTAGE) {
        return;
    }

    // --------------------------------------------------------
    // We already started a cleaning cycle.
    // Wait until it finishes.
    // --------------------------------------------------------
    if (_state == WAITING_FOR_CLEAN_TO_FINISH) {
        if (!_cleaner.isRunning()) {
            _postCleanStartTime = millis();
            _state = POST_CLEAN_SETTLING;
        }
        return;
    }

    // --------------------------------------------------------
    // Cleaning finished. Wait for measurement to settle, then
    // compare voltage before and after cleaning.
    //
    // If output did NOT improve enough, classify the low reading
    // as lighting/weather rather than dirt and stop auto-cleaning.
    // --------------------------------------------------------
    if (_state == POST_CLEAN_SETTLING) {
        if (millis() - _postCleanStartTime < POST_CLEAN_SETTLE_MS) {
            return;
        }

        _postCleanVoltage = currentVoltage;

        if (_preCleanVoltage > MIN_VALID_PANEL_VOLTAGE) {
            _lastImprovementPercent =
                ((_postCleanVoltage - _preCleanVoltage) /
                 _preCleanVoltage) * 100.0f;
        } else {
            _lastImprovementPercent = 0.0f;
        }

        if (_lastImprovementPercent < MIN_CLEANING_IMPROVEMENT_PERCENT) {
            // No useful improvement:
            // assume cloud / shade / sun-angle / lighting condition.
            _state = LIGHT_CONDITION_LOCKOUT;
        } else {
            // Cleaning helped. Resume normal monitoring.
            _state = MONITORING;
        }

        _lowVoltageStartTime = 0;
        return;
    }

    // --------------------------------------------------------
    // Light-condition lockout:
    // do NOT clean again while output remains depressed.
    //
    // Unlock only when voltage recovers close enough to baseline.
    // Example:
    // threshold = 12%, recovery margin = 4%
    // unlock once drop falls below 8%.
    // --------------------------------------------------------
    if (_state == LIGHT_CONDITION_LOCKOUT) {
        const float recoveryThreshold =
            max(0.0f,
                _dropThresholdPercent -
                LIGHT_CONDITION_RECOVERY_MARGIN_PERCENT);

        if (calculateDropPercent(currentVoltage) <= recoveryThreshold) {
            _state = MONITORING;
            _lowVoltageStartTime = 0;
        }

        return;
    }

    // --------------------------------------------------------
    // Normal monitoring
    // --------------------------------------------------------
    if (_baselineVoltage < MIN_VALID_PANEL_VOLTAGE) {
        return;
    }

    const float drop = calculateDropPercent(currentVoltage);

    if (drop >= _dropThresholdPercent) {
        if (_lowVoltageStartTime == 0) {
            _lowVoltageStartTime = millis();
        }

        const bool confirmed =
            millis() - _lowVoltageStartTime >= DROP_CONFIRMATION_TIME_MS;

        if (confirmed && cooldownFinished() && !_cleaner.isRunning()) {
            _preCleanVoltage = currentVoltage;

            _cleaner.clean(
                DEFAULT_WIPE_COUNT,
                DEFAULT_DISPENSE_TIME_MS
            );

            _lastAutoCleanTime = millis();
            _state = WAITING_FOR_CLEAN_TO_FINISH;
            _lowVoltageStartTime = 0;
        }
    } else {
        _lowVoltageStartTime = 0;
    }
}

bool AutoCleanManager::isLightConditionLocked() const {
    return _state == LIGHT_CONDITION_LOCKOUT;
}

float AutoCleanManager::getPreCleanVoltage() const {
    return _preCleanVoltage;
}

float AutoCleanManager::getPostCleanVoltage() const {
    return _postCleanVoltage;
}

float AutoCleanManager::getLastImprovementPercent() const {
    return _lastImprovementPercent;
}

const char* AutoCleanManager::getStateText() const {
    switch (_state) {
        case MONITORING:
            return "Monitoring";

        case WAITING_FOR_CLEAN_TO_FINISH:
            return "Auto cleaning";

        case POST_CLEAN_SETTLING:
            return "Evaluating cleaning result";

        case LIGHT_CONDITION_LOCKOUT:
            return "Lighting condition inferred";

        default:
            return "Unknown";
    }
}
