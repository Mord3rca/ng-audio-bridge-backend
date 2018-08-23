### Authentication

This API provide a read only indexation service so no authentication is required to use it.

### Methods

#### `GET /api/track/<id>`

If the ID exist in the database, the backend server will answer with a 301 Moved Permanently containing the URL of the Track.

#### `GET /api/genres`

Get the genre list in TXT format. Add "?json" at the end of the URI to get it in JSON, the JSON array will be terminated with a null object.

#### `GET /api/version`

Get the version of the software

#### `POST /api/filter/`

Get at max 25 Track info matching your filter, return a JSON Array which contain Tracks info

##### Filter Variable (JSON)

Name            | Type                | Description                                             | default value
----------------|---------------------|---------------------------------------------------------|-------------
minScore        | Float               | Set the minimum score                                   | 0
maxScore        | Float               | Set the maximum score                                   | 5
minDate         | Date String         | Set the minimum submission date                         | "2003/01/01"
maxDate         | Date String         | Set the maximum submission date                         | ""
disableUnrated  | Check Only Presence | If exist, this will prevent unrated Track to be listed  | Do not exist
allowedGenre    | Array of int        | Array of genre's id allowed                             | All selected

#### `POST /api/filter/old`

Compatibility with [NG Audio Bridge](https://www.newgrounds.com/portal/view/553053), this request will get an JSON array of 25 Track max matching your JSON filter.

##### Filter Variable (JSON)

Name            | Type                | Description                                             | default value
----------------|---------------------|---------------------------------------------------------|-------------
minScore        | Float               | Set the minimum score                                   | 0
maxScore        | Float               | Set the maximum score                                   | 5
minDate         | Date String         | Set the minimum submission date                         | "2003/01/01"
maxDate         | Date String         | Set the maximum submission date                         | ""
genres          | Array of string     | Array of genre's string allowed                         | All selected

#### `POST /api/filter/composer`

This will list all Track of a composer. The composer name should be in the "composer" variable of the POST request.
