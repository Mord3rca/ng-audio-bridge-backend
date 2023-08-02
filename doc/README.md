# Audio Bridge API

Documentation of all REST endpoints / object type / ...

Except for compatibility endpoints, the API is expecting a JSON payload.

## Endpoints

### Compatibility

| URI                                | Method                         | Description                                         |
| ---------------------------------- | ------------------------------ | --------------------------------------------------- |
| /crossdomain.xml                   | GET                            | Get a builtin crossdomain for old flash application |
| /audio/serve/\<unused\>/\<id\>.mp3 | GET                            | Get audio file location                             |
| /Radio2/FilterBridge.php           | [POST](radio2/filterbridge.md) | Original filter endpoint, here for compatibility    |

### Info

| URI          | Method                      | Description                         |
| ------------ | --------------------------- | ----------------------------------- |
| /api/genres  | [GET](api/genres/get.md)    | Get an array of all possible genres |
| /api/version | [GET](api/version.md)       | Get API version                     |

### Track

| URI                 | Method                  | Description                 |
| ------------------- | ----------------------- | --------------------------- |
| /api/track/\<id\>   | [GET](api/track/get.md) | Get audio file location     |
| /api/track/random   | [GET](api/track/get.md) | Get a rondam track location |

### Filters

| URI                  | Method                               | Description                  |
| -------------------- | ------------------------------------ | ---------------------------- |
| /api/filter          | [GET](api/filter/get.md)             | Main API filter              |
| /api/filter/composer | [GET](api/filter/composer/get.md)    | Get all song from a composer |
