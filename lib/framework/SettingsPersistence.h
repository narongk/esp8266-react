#ifndef SettingsPersistence_h
#define SettingsPersistence_h

#include <SettingsService.h>
#include <SettingsSerializer.h>
#include <SettingsDeserializer.h>
#include <FS.h>

#define MAX_FILE_SIZE 1024

/**
 * SettingsPersistance takes care of loading and saving settings when they change.
 *
 * SettingsPersistence automatically registers writeToFS as an update handler with the settings manager
 * when constructed, saving any updates to the file system.
 */
template <class T>
class SettingsPersistence {
 public:
  SettingsPersistence(SettingsSerializer<T>* settingsSerializer,
                      SettingsDeserializer<T>* settingsDeserializer,
                      SettingsService<T>* settingsManager,
                      FS* fs,
                      char const* filePath) :
      _settingsSerializer(settingsSerializer),
      _settingsDeserializer(settingsDeserializer),
      _settingsService(settingsManager),
      _fs(fs),
      _filePath(filePath) {
    enableAutomatic();
  }

  void readFromFS() {
    File settingsFile = _fs->open(_filePath, "r");

    if (settingsFile) {
      if (settingsFile.size() <= MAX_FILE_SIZE) {
        DynamicJsonDocument jsonDocument = DynamicJsonDocument(MAX_FILE_SIZE);
        DeserializationError error = deserializeJson(jsonDocument, settingsFile);
        if (error == DeserializationError::Ok && jsonDocument.is<JsonObject>()) {
          readSettings(jsonDocument.as<JsonObject>());
          settingsFile.close();
          return;
        }
      }
      settingsFile.close();
    }

    // If we reach here we have not been successful in loading the config,
    // hard-coded emergency defaults are now applied.
    readDefaults();
  }

  bool writeToFS() {
    // create and populate a new json object
    DynamicJsonDocument jsonDocument = DynamicJsonDocument(MAX_FILE_SIZE);
    _settingsService->read(
        [&](T& settings) { _settingsSerializer->serialize(settings, jsonDocument.to<JsonObject>()); });

    // serialize it to filesystem
    File settingsFile = _fs->open(_filePath, "w");

    // failed to open file, return false
    if (!settingsFile) {
      return false;
    }

    // serialize the data to the file
    serializeJson(jsonDocument, settingsFile);
    settingsFile.close();
    return true;
  }

  void disableAutomatic() {
    if (_updateHandlerId) {
      _settingsService->removeUpdateHandler(_updateHandlerId);
      _updateHandlerId = 0;
    }
  }

  void enableAutomatic() {
    if (!_updateHandlerId) {
      _updateHandlerId = _settingsService->addUpdateHandler([&](String originId) { writeToFS(); });
    }
  }

 private:
  SettingsSerializer<T>* _settingsSerializer;
  SettingsDeserializer<T>* _settingsDeserializer;
  SettingsService<T>* _settingsService;
  FS* _fs;
  char const* _filePath;
  update_handler_id_t _updateHandlerId = 0;

  // read the settings, but do not call propogate
  void readSettings(JsonObject root) {
    _settingsService->read([&](T& settings) { _settingsDeserializer->deserialize(settings, root); });
  }

 protected:
  // We assume the readFromJsonObject supplies sensible defaults if an empty object
  // is supplied, this virtual function allows that to be changed.
  virtual void readDefaults() {
    DynamicJsonDocument jsonDocument = DynamicJsonDocument(MAX_FILE_SIZE);
    readSettings(jsonDocument.to<JsonObject>());
  }
};

#endif  // end SettingsPersistence
