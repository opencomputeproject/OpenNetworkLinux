"""__init__.py

Module init for onlp.onlplib
"""

import ctypes

# properly belongs in AIM.aim_list

class list_links(ctypes.Structure):
    pass

list_links._fields_ = [("prev", ctypes.POINTER(list_links),),
                       ("next", ctypes.POINTER(list_links),),]

class ListIterator(object):

    def __init__(self, links, castType=None):
        self.links = links
        self.castType = castType
        self.cur = self.links.links.next

    def next(self):

        # Hurr, pointer()/POINTER() types are not directly comparable
        p1 = ctypes.cast(self.cur, ctypes.c_void_p)
        p2 = ctypes.cast(self.links.links.prev, ctypes.c_void_p)
        if p1.value == p2.value:
            raise StopIteration

        cur, self.cur = self.cur, self.cur.contents.next
        if self.castType is not None:
            cur = ctypes.cast(cur, ctypes.POINTER(self.castType))
        return cur.contents

class list_head(ctypes.Structure):
    _fields_ = [("links", list_links,),]

    links_klass = list_links

    def __iter__(self):
        if self.links_klass == list_links:
            return ListIterator(self)
        else:
            return ListIterator(self, castType=self.links_klass)

class onlp_onie_vx(list_links):
    # NOTE that Python inheritence merges the fields
    # with the base class (ctypes-ism)
    _fields_ = [("data", ctypes.c_ubyte * 256,),
                ("size", ctypes.c_int,),]

class onlp_onie_vx_list_head(list_head):
    links_klass = onlp_onie_vx

class onlp_onie_info(ctypes.Structure):
    _fields_ = [("product_name", ctypes.c_char_p,),
                ("part_number", ctypes.c_char_p,),
                ("serial_number", ctypes.c_char_p,),
                ("mac", ctypes.c_ubyte * 6,),
                ("manufacture_date", ctypes.c_char_p,),
                ("device_version", ctypes.c_ubyte,),
                ("label_revision", ctypes.c_char_p,),
                ("platform_name", ctypes.c_char_p,),
                ("onie_version", ctypes.c_char_p,),
                ("mac_range", ctypes.c_ushort,),
                ("manufacturer", ctypes.c_char_p,),
                ("country_code", ctypes.c_char_p,),
                ("vendor", ctypes.c_char_p,),
                ("diag_version", ctypes.c_char_p,),
                ("service_tag", ctypes.c_char_p,),
                ("crc", ctypes.c_uint,),

                #
                # Vendor Extensions list, if available.
                #
                ("vx_list", onlp_onie_vx_list_head,),

                # Internal/debug
                ("_hdr_id_string", ctypes.c_char_p,),
                ("_hdr_version", ctypes.c_ubyte,),
                ("_hdr_length", ctypes.c_ubyte,),
                ("_hdr_valid_crc", ctypes.c_ubyte,),]

class onlp_platform_info(ctypes.Structure):
    _fields_ = [("cpld_versions", ctypes.c_char_p,),
                ("other_versions", ctypes.c_char_p,),]
