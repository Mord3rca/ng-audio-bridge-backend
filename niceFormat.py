import urllib

def niceuri(e):
    local = e.replace("\\/", "/")
    pos = local.rfind('?')
    if( pos > 0):
        local = local[:pos]
    return local

def replace_hexcode(e):
    return urllib.parse.unquote(e)
