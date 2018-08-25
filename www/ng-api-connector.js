//Based on answer: https://stackoverflow.com/questions/3066586/get-string-in-yyyymmdd-format-from-js-date-object
Date.prototype.yyyymmdd = function() {
  var mm = this.getMonth() + 1; // getMonth() is zero-based
  var dd = this.getDate();

  return [this.getFullYear(),
          (mm>9 ? '' : '0') + mm,
          (dd>9 ? '' : '0') + dd
         ].join('/');
};

function NG_API_Connector( api_url = document.location ) {
  this._api_url = api_url

  this.filter = function() {
    date = new Date()
    this.maxScore = 5
    this.minScore = 0
    this.allowUnrated = true
    this.minDate = "2003/01/01"
    this.maxDate = date.yyyymmdd()
    this.genres = new Array()
    for(var i = 0;i<48;i++) this.genres[i]=i
  };
  
  /*
  this.oldFilter = function() {
    date = new Date()
    this.maxScore = 5
    this.minScore = 0
    this.minDate = "2003/01/01"
    this.maxDate = date.yyyymmdd()
  };
  */
  
  this.callbacks = new Array()
  
  this.callbacks.onError = function(func, str) { console.warn("[API] NG-Audio ( " + func + ") Error: " + str)  }
  
  this.callbacks.onGenreBegin   = function(){}
  this.callbacks.onGenreNew     = function( genre ){ console.log(JSON.stringify(genre)) }
  this.callbacks.onGenreEnd     = function(){}
  
  this.callbacks.onTracksBegin  = function(){}
  this.callbacks.onTracksNew    = function( track ){ console.log(JSON.stringify(track)) }
  this.callbacks.onTracksEnd    = function(){}
  
  this.callbacks.onComposerTracksBegin  = function(){}
  this.callbacks.onComposerTracksNew    = function( track ){ console.log(JSON.stringify(track)) }
  this.callbacks.onComposerTracksEnd    = function(){}
  
  this.callbacks.onVersion = function( txt ){ console.log("[API] NG-Audio - Version: " + txt ) }
}

NG_API_Connector.prototype.getTrackURL = function ( id ) {
  return this._api_url + "api/track/" + id
}

NG_API_Connector.prototype.getGenreList = function (){
  var begin_callback= this.callbacks.onGenreBegin
  var new_callback  = this.callbacks.onGenreNew
  var end_callback  = this.callbacks.onGenreEnd
  
  var error_callback = this.callbacks.onError
  
  function i( obj_json ) {
    var json = JSON.parse(obj_json)
    
    if( typeof json.genres == 'undefined' )
    {
      error_callback("getGenreList", "Answer does not contain an array named genres")
      return;
    }
    
    begin_callback()
    for( key in json["genres"] ){
      if( typeof json["genres"][key].id !== 'undefined' && json["genres"][key].id !== -1 )
        new_callback(json.genres[key])
    }
    end_callback()
  }
  
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if( this.readyState==4 )
    {
      if( this.status == 200 )
        i( this.responseText );
      else
        error_callback("getGenreList", "Couldn't fetch genre list: HTTP status error " + this.status );
    }
  };
  xhttp.open("GET", this._api_url + "api/genres?json", true);
  xhttp.send();
}

NG_API_Connector.prototype.getVersion = function() {
  var callback = this.callbacks.onVersion
  var error_callback = this.callbacks.onError
  
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if( this.readyState==4 )
    {
      if( this.status == 200 )
        callback( this.responseText );
      else
        error_callback("getVersion", "Couldn't fetch version info: HTTP status error " + this.status )
    }
  };
  xhttp.open("GET", this._api_url + "api/version", true);
  xhttp.send();
}

NG_API_Connector.prototype.getTracks = function( filter ){
  var begin_callback  = this.callbacks.onTracksBegin
  var new_callback    = this.callbacks.onTracksNew
  var end_callback    = this.callbacks.onTracksEnd
  
  var error_callback  = this.callbacks.onError
  
  if(typeof filter === 'undefined') filter = {}
  
  function i( json_str ){
    var json = JSON.parse(json_str)
    
    if( typeof json.Tracks === 'undefined' )
    {
      error_callback("getTracks", "JSON answer not understood.")
      return;
    }
    
    begin_callback()
    for(k in json.Tracks)
      new_callback( json.Tracks[k] )
    end_callback()
  }
  
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if( this.readyState==4 )
    {
      if( this.status == 200 )
        i( this.responseText );
      else
        error_callback("getTracks", "Couldn't fetch tracks: HTTP status error " + this.status )
    }
  };
  xhttp.open("POST", this._api_url + "api/filter/", true);
  
  xhttp.send( "filterObject="+JSON.stringify(filter) );
}

NG_API_Connector.prototype.getComposerTracks = function( composer_name ){
  var begin_callback  = this.callbacks.onComposerTracksBegin
  var new_callback    = this.callbacks.onComposerTracksNew
  var end_callback    = this.callbacks.onComposerTracksEnd
  
  var error_callback  = this.callbacks.onError
  
  function i( json_str ){
    var json = JSON.parse(json_str)
    
    if( typeof json.Tracks === 'undefined' )
    {
      error_callback("getComposerTracks", "JSON answer not understood.")
      return;
    }
    
    begin_callback()
    for(k in json.Tracks)
      new_callback( json.Tracks[k] )
    end_callback()
  }
  
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if( this.readyState==4 )
    {
      if( this.status == 200 )
        i( this.responseText );
      else
        error_callback("getComposerTracks", "Couldn't fetch tracks: HTTP status error " + this.status )
    }
  };
  xhttp.open("POST", this._api_url + "api/filter/composer", true);
  xhttp.send( "composer=" + composer_name );
}
