# POST /Radio2/FilterBridge.php

## Description

Return an array of songs matching your filter

## URL Parameters

  * filterJSON: JSON object containing filter parameters
    * minScore (num): Minimal Score
    * maxScore (num): Maximal Score
    * minDate (string format: "\<YYYY\>/\<MM\>/\<DD\>"): Submission date
    * maxDate (string format: "\<YYYY\>/\<MM\>/\<DD\>"): Submission date
    * genres (Array\<string\>): Array of allowed genres

## Response

A JSON object:

  * ResultSet (Array<[song](../objects/song.md)>): Result

## Example Response

```
{
  "ResultSet": [
    {
      "composer": "Mistasmijen",
      "date": "2005/12/29",
      "genre": "General Rock",
      "id": 39560,
      "score": 4.059999942779541,
      "title": "Wanna be your Dog (iggy Cover)",
      "url": "https://audio.ngfiles.com/39000/39560_newgrounds_wanna_.mp3"
    },
    ...
    {
      "composer": "SkullPioneer",
      "date": "2018/07/23",
      "genre": "Video Game",
      "id": 816642,
      "score": 4.289999961853027,
      "title": "[Remix] Even Care",
      "url": "https://audio.ngfiles.com/816000/816642_Remix-Even-Care.mp3"
    }
  ]
}
```
