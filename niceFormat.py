def niceuri(e):
    local = e.replace("\\/", "/")
    pos = local.rfind('?')
    if( pos > 0):
        local = local[:pos]
    return local

def replace_hexcode(e):
    e = e.replace("%20", ' ').replace("%22", '"').replace("%7B", '{').replace("%7D", '}').replace("%3A", ':').replace("%2C", ',').replace("%2D", '-').replace("", '').replace("", '').replace("%5B", '[').replace("%5D", ']')
    return e
