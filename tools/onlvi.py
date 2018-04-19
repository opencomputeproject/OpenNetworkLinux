import subprocess

class OnlVersionImplementation(object):

    PRODUCTS = [
        {
            "id" : "ONL",
#            "version": "20YY-MM"
        }
    ]

    def __init__(self):
        if 'version' in self.PRODUCTS[0]:
            # Release builds have a specific version.
            self.release = True
        else:
            # The current branch is used as the release version.
            self.release = False
            cmd = ('git', 'rev-parse', '--abbrev-ref', 'HEAD')
            branch = subprocess.check_output(cmd).strip()
            self.PRODUCTS[0]['version'] = branch

    def V_OS_NAME(self, data):
        return "Open Network Linux OS"

    def V_BUILD_SHA1(self, data):
        return data['build_sha1']

    def V_BUILD_SHORT_SHA1(self, data):
        return self.V_BUILD_SHA1(data)[0:7]

    def V_BUILD_TIMESTAMP(self, data):
        return data['build_timestamp']

    def V_FNAME_BUILD_TIMESTAMP(self, data):
        return self.V_BUILD_TIMESTAMP(data).replace(':', '')

    def V_BUILD_ID(self, data):
        return "%s-%s" % (self.V_BUILD_TIMESTAMP(data), self.V_BUILD_SHORT_SHA1(data))

    def V_FNAME_BUILD_ID(self, data):
        return "%s-%s" % (self.V_FNAME_BUILD_TIMESTAMP(data), self.V_BUILD_SHORT_SHA1(data))

    def V_PRODUCT_ID_VERSION(self, data):
        return data['product']['version']

    def V_VERSION_ID(self, data):
        return "ONL-%s" % (self.V_PRODUCT_ID_VERSION(data))

    def V_FNAME_VERSION_ID(self, data):
        return self.V_VERSION_ID(data)

    def V_PRODUCT_VERSION(self, data):
        return "ONL-%s"  % (self.V_PRODUCT_ID_VERSION(data))

    def V_FNAME_PRODUCT_VERSION(self, data):
        return "ONL-%s" % (self.V_PRODUCT_ID_VERSION(data))

    def V_VERSION_STRING(self, data):
        return "%s %s, %s" % (self.V_OS_NAME(data), self.V_VERSION_ID(data), self.V_BUILD_ID(data))

    def V_RELEASE_ID(self, data):
        return "%s,%s" % (self.V_VERSION_ID(data), self.V_BUILD_ID(data))

    def V_FNAME_RELEASE_ID(self, data):
        return "%s-%s" % (self.V_VERSION_ID(data), self.V_FNAME_BUILD_ID(data))

    def V_SYSTEM_COMPATIBILITY_VERSION(self, data):
        return "2"

    def V_ISSUE(self, data):
        if self.release:
            return "%s %s" % (self.V_OS_NAME(data), self.V_VERSION_ID(data))
        else:
            return self.V_VERSION_STRING(data)
