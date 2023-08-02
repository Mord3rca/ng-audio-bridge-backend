# GET /api/tracks/\<id\>

Get the URL of a song

secial case: `id` can be *random* in this case, a random track will be sent.

## Response

### Success

HTTP 301 - Redirecting to the mp3 URL on ngfiles.com

### Failure

HTTP 404
