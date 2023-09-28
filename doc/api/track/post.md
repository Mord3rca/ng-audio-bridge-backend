# POST /api/tracks/\<id\>

Update a song entry

## JSON Parameters

  * title (string): Song name
  * composer (string): Artist name
  * score (float): Song score
  * genre (string): Song genre
  * date (String): Submission date
  * url (string): Song URL

## Response

### Success

HTTP 200 - Song updated

### Failure

HTTP 404
