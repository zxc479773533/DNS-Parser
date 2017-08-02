# DNS-Parser

A simple DNS parser

## Features

This is a simple program to get the IP address of a domain name.

Outputs:

```
Please enter your gateway's ip: (format: 0.0.0.0)
192.168.1.1

Please enter a domain name: (q for exit.)
www.baidu.com

Get Answer:
Domain name: www.baidu.com
Alias name: www.a.shifen.com

Domain name: www.a.shifen.com
IP address: 180.97.33.108
Time to alive: 90

Domain name: www.a.shifen.com
IP address: 180.97.33.107
Time to alive: 90
```

## To achieve

The core part is the algorithm to get the domain name in the compressing tag.

A example that the domain name `www.baidu.com` in the DNS packet:

```
| 3 | w | w | w | 5 | b | a | i | d | u | 3 | c | o | m | 0 |
```

Here is the example that the domain name `tieba.baidu.com` has been compressed:

```
| 3 | t | i | e | b | a | 0xc0 | pos of '5' before baidu |
```

## Others

Actually, this DNS parser can get IP in the DNS packet, right?

Then you can use the function in ARPSpoof, and do some snake operate, ..., or something else, ha?

Who cares?