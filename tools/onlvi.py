class OnlVersionImplementation(object):

    PRODUCTS = [
        {
            "id" : "ONL",
            "version": "2.0.0"
            }
        ]


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
