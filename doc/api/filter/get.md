# GET /api/filter

Get songs list according to a filter

## JSON Parameters

  * minDate (string): Submission date
  * maxDate (string): Submission date
  * minScore (num): Score min of the song
  * maxScore (num): Score max of the song
  * allowUnrated (bool): Allow unrated songs
  * genres (Array\<int\>): Array of [genre](../../objects/genre.md) id allowed

## Response

  * Tracks (Array<[song](../../objects/song.md)>): List of songs matching the filter

## Example Response

```
{
  "Tracks": [
    {
      "composer": "Elfire",
      "date": "2008/09/02",
      "genre": "Industrial",
      "id": 169655,
      "score": 4.130000114440918,
      "title": "Electric Pulse [PDM]",
      "url": "https://audio.ngfiles.com/169000/169655_Real1.mp3"
    },
    ...
    {
      "composer": "Elfire",
      "date": "2011/04/14",
      "genre": "Industrial",
      "id": 413494,
      "score": 4.480000019073486,
      "title": "Elfire - Necro Syncrosis",
      "url": "https://audio.ngfiles.com/413000/413494_Necro_Syncrosis.mp3"
    }
  ]
}
```
