#!/usr/bin/env python3

import sys
import re
from lxml import etree

tree = etree.parse(sys.argv[1])
xpath_expression = sys.argv[2]

#  a hack allowing to access the
#  default namespace (if defined) via the 'p:' prefix
#  E.g. given a default namespaces such as 'xmlns="http://maven.apache.org/POM/4.0.0"'
#  an XPath of '//p:module' will return all the 'module' nodes
ns = tree.getroot().nsmap
if ns.keys() and None in ns:
    ns['p'] = ns.pop(None)
#   end of hack

for e in tree.xpath(xpath_expression, namespaces=ns):
    if isinstance(e, str):
        output = e
    else:
        output = e.text and e.text.strip() or etree.tostring(e, pretty_print=True).decode()

    print(output)
