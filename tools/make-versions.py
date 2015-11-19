#!/usr/bin/python
############################################################
import os
import sys
import argparse
import subprocess
import json
import pprint
import yaml

class OnlVersionsGenerator(object):
    def __init__(self, ops):

        sys.path.append(os.path.dirname(ops.import_file))
        m = __import__(os.path.basename(ops.import_file))

        if hasattr(m, ops.class_name):
            self.implementation = getattr(m, ops.class_name)()
        else:
            raise ValueError("The import file %s does not contain a class named %s" % (ops.import_file, ops.class_name))

        self.ops = ops
        self.build_sha1 = subprocess.check_output("git rev-list HEAD -1", shell=True).strip()
        self.build_timestamp =subprocess.check_output("date +%Y-%m-%d.%H:%M", shell=True).strip()

    def generate_all(self):
        for product in self.implementation.PRODUCTS:
            for config in product.get('builds', [None]):
                data = {}
                data['product'] = product
                data['build_timestamp'] = self.build_timestamp
                data['build_sha1'] = self.build_sha1
                if config is not None:
                    data['build_config'] = config
                self.generate(data)

    def generate(self, d):
        data = {}
        directory = dir(self.implementation)
        for attribute in directory:
            if attribute.startswith("V_"):
                key = attribute.replace("V_", "")
                data[key] = getattr(self.implementation, attribute)(d)


        if not os.path.isdir(self.ops.output_dir):
            os.makedirs(ops.output_dir)

        basename = "version-%s" % (d['product']['id'].lower())
        if 'build_config' in d:
            basename += "-%s" % d['build_config']

        # JSON
        fname = os.path.join(self.ops.output_dir, basename + '.json')
        if not os.path.exists(fname) or self.ops.force:
            with open(fname, "w") as f:
                json.dump(data, f, indent=2)

        # mk
        fname = os.path.join(self.ops.output_dir, basename + '.mk')
        if not os.path.exists(fname) or self.ops.force:
            with open(fname, "w") as f:
                for k in sorted(data.keys()):
                    f.write("%s%s=%s\n" % ("export " if self.ops.export else "", k, data[k]))
                f.write("\n")

        # sh
        fname = os.path.join(self.ops.output_dir, basename + '.sh')
        if not os.path.exists(fname) or self.ops.force:
            with open(fname, "w") as f:
                for k in sorted(data.keys()):
                    f.write("%s%s='%s'\n" % ("export " if self.ops.export else "", k, data[k]))
                f.write("\n")

        if self.ops.print_:
            pprint.pprint(data, indent=2)


if __name__ == '__main__':
    import argparse

    ap = argparse.ArgumentParser(description="ONL Version Tool")
    ap.add_argument("--import-file", required=True, help="The path the python file containing the implementation class.")
    ap.add_argument("--class-name", required=True, help="The name of the implementation class.")
    ap.add_argument("--output-dir", required=True, help="Location for generated files.")
    ap.add_argument("--force", action='store_true', help="Force regeneration.")
    ap.add_argument("--export", action='store_true', help="Include export keyword in .sh and .mk versions.")
    ap.add_argument("--print", action='store_true', help="Print version data.", dest='print_')
    ops = ap.parse_args()

    o = OnlVersionsGenerator(ops)
    o.generate_all()




