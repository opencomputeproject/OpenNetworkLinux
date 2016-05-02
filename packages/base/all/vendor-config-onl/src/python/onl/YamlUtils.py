"""YamlUtils.py

"""

import yaml
try:
    import onlyaml
    def load(stream):
        return yaml.load(stream, Loader=onlyaml.Loader)
except ImportError:
    load = yaml.load

def merge(p1, p2):
    """Merge two YAML files.

    y1 is the 'default' source; leaf values from y2 will override.
    Return the merged tree.

    y1 should be a dict with a single top-level key, 'default'.
    y2 should be a dict with a single top-level key, not 'default'.

    Set a leaf in y2 to nil ('~') to create a tombstone (discard any key
    from y1).

    if a (sub) key in y1, y2 differ in type (dict vs. non-dict) then
    the merge will proceed with the non-dict promoted to a dict using
    the default-key schema ('='). Consumers of this function should be
    prepared to handle such keys.
    """

    with open(p1) as fd:
        buf1 = fd.read()
    with open(p2) as fd:
        buf2 = fd.read()

    # read p1 as-is, make sure it looks like a 'default' YAML
    c1 = load(buf1)
    k1 = list(c1.keys())
    if k1 != ['default']:
        raise ValueError("%s: invalid top-level keys for default mapping: %s"
                         % (p1, k1,))

    # read p2 with the default YAML as a sub-key (to resolve anchors)
    lines = buf2.splitlines(False)
    lines = [x for x in lines if x != '---']
    buf3 = buf1 + "\n" + "\n".join(lines)
    c2 = load(buf3)
    c2.pop('default', None)

    k2 = list(c2.keys())
    if len(k2) != 1:
        raise ValueError("invalid format for target mapping")
    tgtKey = k2[0]

    merged = { tgtKey : {} }
    q = [(c1['default'], c2[tgtKey], merged[tgtKey])]
    while True:
        if not q: break
        c1, c2, c3 = q.pop(0)
        # add in non-overlapping keys
        # 'None' keys from p2 are tombstones
        s1 = set(c1.keys())
        s2 = set(c2.keys())

        for k in s1.difference(s2):
            v = c1[k]
            if type(v) == dict:
                c3.setdefault(k, {})
                q.append((v, {}, c3[k],))
            else:
                c3.setdefault(k, v)

        for k in s2.difference(s1):
            v = c2[k]
            if v is None: continue
            if type(v) == dict:
                c3.setdefault(k, {})
                q.append(({}, v, c3[k],))
            else:
                c3.setdefault(k, v)

        # handle overlapping keys
        for k in s1.intersection(s2):
            v1 = c1[k]
            v2 = c2[k]

            if v2 is None: continue

            # two dicts, key-by-key reconciliation required
            if type(v1) == dict and type(v2) == dict:
                c3.setdefault(k, {})
                q.append((v1, v2, c3[k],))
                continue

            # two non-dicts, p2 wins
            if type(v1) != dict and type(v2) != dict:
                c3[k] = v2
                continue

            if type(v1) != dict:
                v1 = { '=' : v1, }
            if type(v2) != dict:
                v2 = { '=' : v2, }
            c3.setdefault(k, {})
            q.append((v1, v2, c3[k],))

    return merged
