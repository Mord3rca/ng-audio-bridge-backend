# PUT /api/tracks/

Add a song entry

## JSON Parameters

  * id (int): Songs ID on Newground
  * title (string): Song name
  * composer (string): Artist name
  * score (float): Song score
  * genre (string): Song genre
  * date (String): Submission date
  * url (string): Song URL

## Response

### Success

HTTP 200 - Song was added

### Failure

HTTP 404
