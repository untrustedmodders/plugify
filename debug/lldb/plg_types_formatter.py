"""
=============================================================================
plugify
Copyright (C) 2023-2025 untrustedmodders
=============================================================================

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 3.0, as published by the
Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.
"""

import lldb

def __lldb_init_module(debugger, dict):
    #todo
    debugger.HandleCommand('type synthetic add -x "^plg::basic_string<.+>$" --python-class plg_types_formatter.StringProvider')
    debugger.HandleCommand('type synthetic add -x "^plg::vector<.+>$" --python-class plg_types_formatter.VectorProvider')
    debugger.HandleCommand('type synthetic add -x "^plg::variant<.+>$" --python-class plg_types_formatter.VariantProvider')
    debugger.HandleCommand('type synthetic add -x "^plg::any$" --python-class plg_types_formatter.VariantProvider')
    debugger.HandleCommand('type summary add -x "^plg::basic_string<.+>$" --summary-string "${svar}"')
    debugger.HandleCommand('type summary add -x "^plg::vector<.+>$" --summary-string "${svar}"')
    debugger.HandleCommand('type summary add -x "^plg::variant<.+>$" --summary-string "${svar}"')
    debugger.HandleCommand('type summary add -x "^plg::any$" --summary-string "${svar}"')
    print("✅ plg::types formatter LOADED!")

class StringProvider:
    """Custom formatter for plg::string"""

    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self.update()

    def get_value(self):
        """Return the string value for summary display"""
        return f'"{self.string}"'

    def update(self):
        try:
            # Access the union rep_
            rep = self.valobj.GetChildMemberWithName('rep_')
            short_rep = rep.GetChildMemberWithName('s')
            cap = short_rep.GetChildMemberWithName('spare_size_')
            data = cap.GetData()
            error = lldb.SBError()
            raw_byte = data.GetUnsignedInt8(error, 0)
            spare_size = raw_byte & 0x7F
            is_long = (raw_byte >> 7) & 0x1

            self.is_long = is_long

            if self.is_long:
                # Long string - access rep_.l
                long_rep = rep.GetChildMemberWithName('l')
                data_ptr = long_rep.GetChildMemberWithName('data_')
                self.size = long_rep.GetChildMemberWithName('size_').GetValueAsUnsigned(0)
                self.capacity = long_rep.GetChildMemberWithName('cap_').GetValueAsUnsigned(0)

                # Read string from heap pointer
                self.string = ""
                if data_ptr.GetValueAsUnsigned(0) != 0:
                    data = data_ptr.GetPointeeData(0, self.size)
                    for i in range(self.size):
                        byte = data.GetUnsignedInt8(error, i)
                        if 32 <= byte < 127:  # Printable ASCII
                            self.string += chr(byte)
                        else:
                            self.string += '?'  # Non-printable characters
                else:
                    self.string = ""
            else:
                # Short string (SSO) - access rep_.s
                # min_cap = 24, so buffer size is 23
                self.capacity = 23
                self.size = self.capacity - spare_size

                # Read from inline data_ buffer
                self.string = ""
                data_array = short_rep.GetChildMemberWithName('data_')

                for i in range(min(self.size, 23)):
                    child = data_array.GetChildAtIndex(i)
                    if child.IsValid():
                        char_val = child.GetValueAsUnsigned(0)
                        if char_val == 0:
                            break
                        if 32 <= char_val < 127:  # Printable ASCII
                            self.string += chr(char_val)
                        elif char_val < 128:
                            self.string += chr(char_val)
                        else:
                            self.string += '?'
                    else:
                        break

        except Exception as e:
            print(f"EXCEPTION: {e}")
            import traceback
            traceback.print_exc()

            self.string = "<error: " + str(e) + ">"
            self.size = 0
            self.capacity = 0
            self.is_long = False

    def num_children(self):
        return 3  # value, size, capacity

    def get_child_at_index(self, index):
        if index < 0 or index >= self.num_children():
            return None
        try:
            if index == 0:
                return self.valobj.CreateValueFromExpression(
                    'value',
                    '"' + self.string + '"'
                )
            elif index == 1:
                return self.valobj.CreateValueFromExpression(
                    'size',
                    'static_cast<size_t>(' + str(self.size) + ')'
                )
            elif index == 2:
                return self.valobj.CreateValueFromExpression(
                    'capacity',
                    'static_cast<size_t>(' + str(self.capacity) + ')'
                )
        except:
            return None

class VectorProvider:
    """Custom formatter for plg::vector"""

    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self.update()

    def update(self):
        try:
            self.begin = self.valobj.GetChildMemberWithName('begin_')
            self.end = self.valobj.GetChildMemberWithName('end_')
            self.cap = self.valobj.GetChildMemberWithName('cap_')

            # Get the element type
            self.data_type = self.begin.GetType().GetPointeeType()
            self.type_size = self.data_type.GetByteSize()

            # Calculate size and capacity
            begin_addr = self.begin.GetValueAsUnsigned(0)
            end_addr = self.end.GetValueAsUnsigned(0)
            cap_addr = self.cap.GetValueAsUnsigned(0)

            if self.type_size > 0:
                self.size = (end_addr - begin_addr) // self.type_size
                self.capacity = (cap_addr - begin_addr) // self.type_size
            else:
                self.size = 0
                self.capacity = 0
        except Exception as e:
            print(f"EXCEPTION: {e}")
            import traceback
            traceback.print_exc()

            self.size = 0
            self.capacity = 0

    def num_children(self):
        return 2 + self.size  # size, capacity, + elements

    def get_child_at_index(self, index):
        if index < 0 or index >= self.num_children():
            return None
        try:
            if index == 0:
                # Size
                return self.valobj.CreateValueFromExpression(
                    'size',
                    'static_cast<size_t>(' + str(self.size) + ')'
                )
            elif index == 1:
                # Capacity
                return self.valobj.CreateValueFromExpression(
                    'capacity',
                    'static_cast<size_t>(' + str(self.capacity) + ')'
                )
            else:
                # Array element
                element_index = index - 2
                offset = element_index * self.type_size
                return self.begin.CreateChildAtOffset(
                    '[' + str(element_index) + ']',
                    offset,
                    self.data_type
                )
        except:
            return None

    def has_children(self):
        return True

# Map index to type name in your variant
PLG_ANY_TYPES = [
    "plg::invalid", "plg::none", "bool", "char", "char16_t", "int8_t", "int16_t",
    "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "void*", "float", "double", "plg::function", "plg::string", "plg::variant<plg::none>",
    "std::vector<bool>", "std::vector<char>", "std::vector<char16_t>", "std::vector<int8_t>",
    "std::vector<int16_t>", "std::vector<int32_t>", "std::vector<int64_t>", "std::vector<uint8_t>",
    "std::vector<uint16_t>", "std::vector<uint32_t>", "std::vector<uint64_t>", "std::vector<void*>",
    "std::vector<float>", "std::vector<double>", "std::vector<plg::string>", "std::vector<plg::variant<plg::none>>",
    "std::vector<plg::vec2>", "std::vector<plg::vec3>", "std::vector<plg::vec4>", "std::vector<plg::mat4x4>",
    "plg::vec2", "plg::vec3", "plg::vec4"
]

class VariantProvider:
    """Custom formatter for plg::variant"""

    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self.update()

    def num_children(self):
        return 1 if 0 <= self.index < len(PLG_ANY_TYPES) else 0

    def get_child_at_index(self, idx):
        if idx != 0:
            return None
        if not (0 <= self.index < len(PLG_ANY_TYPES)):
            return None

        type_name = PLG_ANY_TYPES[self.index]
        # Cast storage to the active type
        child_val = self.storage.Cast(self.storage.GetTarget().FindFirstType(type_name))
        #child_val.SetName(type_name)
        return child_val

    def has_children(self):
        return self.num_children() > 0

    def update(self):
        try:
            self.index = self.valobj.GetChildMemberWithName("current_").GetValueAsUnsigned()
            self.storage = self.valobj.GetChildMemberWithName("storage_")

        except Exception as e:
            print(f"EXCEPTION: {e}")
            import traceback
            traceback.print_exc()

            self.index = 0
            self.storage = 0

#
# class VariantProvider:
#     """Custom formatter for plg::variant"""
#
#     def __init__(self, valobj, internal_dict):
#         self.valobj = valobj
#         self.update()
#
#     def update(self):
#         try:
#             # Index of the currently active type
#             self.current_index = self.valobj.GetChildMemberWithName('_current').GetValueAsUnsigned(0)
#
#             # Access the union storage
#             self.storage = self.valobj.GetChildMemberWithName('_storage')
#
#             # Flatten the union tree
#             self.children = []
#             union_node = self.storage
#             while union_node.IsValid() and union_node.GetNumChildren() > 0:
#                 # For a detail::union_node, the first child is left, second is right
#                 left = union_node.GetChildAtIndex(0)
#                 right = union_node.GetChildAtIndex(1)
#                 self.children.append(left)
#                 union_node = right
#
#             # Select the active value
#             if self.current_index < len(self.children):
#                 self.active = self.children[self.current_index]
#             else:
#                 self.active = None
#
#         except Exception as e:
#             print(f"EXCEPTION: {e}")
#             import traceback
#             traceback.print_exc()
#
#             self.active = None
#             self.children = []
#             self.current_index = 0
#
#     def num_children(self):
#         # expose current index and active value
#         return 2
#
#     def get_child_at_index(self, index):
#         if index == 0:
#             return self.valobj.CreateValueFromExpression('current_index', str(self.current_index))
#         elif index == 1 and self.active is not None:
#             return self.active
#         return None
#
#     def get_value(self):
#         if self.active is not None:
#             return self.active.GetSummary() or "<active>"
#         return "<empty>"
#
#     def has_children(self):
#         return True
