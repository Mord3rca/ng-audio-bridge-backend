# GET /api/genres

Get supported genres

## Response

JSON Object:
  * genres (list<[genre](../../objects/genre.md)>): Supported genre

## Example Response

```
{
  "genres": [
    {
      "id": -1,
      "name": "Unknown"
    },
    {
      "id": 0,
      "name": "Classical"
    },
    ...
    {
      "id": 47,
      "name": "Voice Demo"
    }
}
```
