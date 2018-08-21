PRAGMA foreign_keys = ON;

CREATE INDEX Tracks_Index ON Tracks(genre, score, submission_date, composer);
