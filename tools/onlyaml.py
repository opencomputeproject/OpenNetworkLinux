#!/usr/bin/python
############################################################
#
# Extended YAML Support
#
# Supports include files and variable interpolations.
#
############################################################
import yaml
import os
import pprint
import tempfile
from string import Template

class OnlYamlError(Exception):
    """General Error Exception"""
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return self.value


def dflatten(rv, in_):
    if type(in_) is dict:
        rv.update(in_)
        return rv
    elif type(in_) is list:
        for e in in_:
            dflatten(rv, e)
        return rv
    else:
        raise OnlYamlError("Element type '%s' cannot be added to the given dictionary." % (type(in_)))


def loadf(fname, vard={}):

    # Apply variable interpolation:
    def interpolate(s, d):

        error_string = "Yaml variable substitution error: '%s' could not be resolved."

        try:
            template = Template(s)
            s = template.substitute(d)
        except KeyError as e:
            raise OnlYamlError(error_string % (e.args[0]))
        except:
            raise

        return s

    variables = {}

    # Files can reference environment variables
    variables.update(os.environ)

    # Files can reference their own directory.
    variables['__DIR__'] = os.path.dirname(os.path.abspath(fname))

    # Files can reference invokation parameters.
    variables.update(vard)

    # Yaml Include constructor. Allows variables.
    def onlyaml_include(loader, node):
        # Get the path out of the yaml file
        directive = node.value
        fields = directive.split()
        fname = fields[0]
        options = fields[1:]

        for opt in options:
            try:
                (k,v) = opt.split('=')
            except ValueError:
                raise OnlYamlError("Bad include directive: %s" % opt)
            variables[k] = v;

        fname = interpolate(fname, variables)

        if not os.path.isabs(fname):
            fname = os.path.join(os.path.dirname(loader.name), fname)

        if not os.path.exists(fname):
            raise OnlYamlError("Include file '%s' (from %s) does not exist." % (fname, loader.name))

        return loadf(fname, variables)

    # Yaml dynamic constructor. Allow dynamically generated yaml.
    def onlyaml_script(loader, node):
        directive = interpolate(node.value, variables)
        tf = tempfile.NamedTemporaryFile()
        tf.close()
        if os.system("%s > %s" % (directive, tf.name)) != 0:
            raise OnlYamlError("Script execution '%s' failed." % directive)
        return loadf(tf.name, variables)


    yaml.add_constructor("!include", onlyaml_include)
    yaml.add_constructor("!script", onlyaml_script)

    # First load: grab the variables dict
    string = open(fname).read()
    try:
        data = yaml.load(string)
    except Exception, e:
        raise OnlYamlError("%s\n(filename: %s)" % (e, fname))

    if type(data) is dict:
        _v = dflatten({}, data.get('variables', {}))
        variables.update(_v)

        for (k,v) in _v.iteritems():
            k = interpolate(k, variables)
            v = interpolate(v, variables)
            variables[k] = v


    ############################################################
    #
    # Interpolate the entire package contents using the
    # generated variables dict and reload it.
    #
    ############################################################
    string = interpolate(string, variables)

    try:
        data = yaml.load(string)
    except OnlYamlError, e:
        raise e
    except Exception, e:
        raise OnlYamlError("Interpolation produced invalid results:\n%s\n" %  string)

    return data


if __name__ == '__main__':
    import sys
    try:
        if len(sys.argv) == 2:
            print yaml.dump(loadf(sys.argv[1]))
        else:
            sys.stderr.write("usage: %s <yamlfile>\n" % sys.argv[0])
    except OnlYamlError, e:
        sys.stderr.write("error: %s\n" % e.value)


