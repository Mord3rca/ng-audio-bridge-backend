function NG_API_Connector( api_url ) {
  this._api_url = api_url;

  this.filter = function() {
    this.maxScore = 5
    this.minScore = 0
    this.allowUnrated = true
    this.minDate = "2003/01/01"
    this.maxDate = Date.getFullYear() + "/" + Date.getMonth()+1 + "/" + Date.getDate()
    this.genres = new Array()
  };
  
  this.callbacks = new Array()
  
  this.callbacks.onGenreInit   = function(){}
  this.callbacks.onGenreFetch  = function( genre ){ console.log(JSON.stringify(genre)) }
  this.callbacks.onGenreFinish = function(){}
  
  this.callbacks.onVersion = function( txt ){ console.log("[API] NG-Audio - " + txt ) }
}

NG_API_Connector.prototype.getTrackURL = function ( id ) {
  return this._api_url + "/api/track/" + id
}

NG_API_Connector.prototype.fetchGenreList = function (){
  var clear_callback= this.callbacks["onGenreInit"]
  var new_callback  = this.callbacks["onGenreFetch"]
  var end_callback  = this.callbacks["onGenreFinish"]
  
  function i( obj_json ) {
    console.log("[API] NG-Audio: Parsing genre list")
    var json = JSON.parse(obj_json)
    
    if( typeof json.genres == 'undefined' )
      return;
    
    clear_callback()
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
        console.log("[API] NG-Audio: Couldn't fetch genre list: HTTP status error " + this.status );
    }
  };
  xhttp.open("GET", this._api_url + "/api/genres?json", true);
  xhttp.send();
}

NG_API_Connector.prototype.getVersion = function() {
  var callback = this.callbacks["onVersion"]
  
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if( this.readyState==4 )
    {
      if( this.status == 200 )
        callback( this.responseText );
      else
        console.log("[API] NG-Audio: Couldn't fetch version info: HTTP status error " + this.status );
    }
  };
  xhttp.open("GET", this._api_url + "/api/version", true);
  xhttp.send();
}
