#! /usr/bin/python3

#f = open("manuf-small", "r");
f = open("manuf", "r");

structstr = """
struct _macvendor
{
    uint64_t    macid;
    char *      name;
};

struct _macvendor macvendorlist[] =
{
"""

for line in f:
    #print (line)
    if line[0] == '#':
        continue

    nt = line.find('\t')
    if nt < 8:
        continue
    if nt > 20:
        print ("SHOULD NOT HAPPEN")
        continue
    mac = line[0:nt]
    ns = mac.find('/')
    if ns > 0:
        mac = mac[0:ns]
    #print (mac)

    if len(mac) > 18:
        print ("SHOULD NOT HAPPEN")
        continue

    n = 0
    val = 0
    MacHex = "0x"
    while (n < 4):

        if line[(n+1)*3-1] == ':':
            MacHex += mac[n*3:(n+1)*3-1]
            #print (mac[n*3:(n+1)*3-1])
        else:
            break
            
        n = n + 1
    
    #print (mac[n*3:(n+1)*3-1])
    MacHex += mac[n*3:(n+1)*3-1]
    while (n < 4):
        #print ("00")
        MacHex += "00"
        n = n + 1

    nv = line[nt+1:].find('\t')

    if (nv > 0):
        vendor = line[nt+1+nv+1:] # Long string is available
    else:
        vendor = line[nt+1:]    # only small name is available

    # Remove multple whitespaces
    vendor = " ".join(vendor.split())
    # and make quotes safe for C...
    vendor = vendor.replace('"', '')

    structstr += " {" + MacHex + ", \"" + vendor.rstrip()[:63] + "\"},\n"

structstr += """};
"""

print (structstr)

