# Basic Type mapping

The following lists how the types are exposed to the C++ API.

| Type                       | Alias    | Ref ? |
|----------------------------|----------|-------|
| void                       | void     | false |
| bool                       | bool     | true  |
| char                       | char8    | true  |
| char16_t                   | char16   | true  |
| int8_t                     | int8     | true  |
| int16_t                    | int16    | true  |
| int32_t                    | int32    | true  |
| int64_t                    | int64    | true  |
| uint8_t                    | uint8    | true  |
| uint16_t                   | uint16   | true  |
| uint32_t                   | uint32   | true  |
| uint64_t                   | uint64   | true  |
| uintptr_t                  | ptr64    | true  |
| uintptr_t                  | ptr32    | true  |
| float                      | float    | true  |
| double                     | double   | true  |
| void*                      | function | false |
| plg::string                | string   | true  |
| plg::vector\<bool\>        | bool*    | true  |
| plg::vector\<char\>        | char8*   | true  |
| plg::vector\<char16_t\>    | char16*  | true  |
| plg::vector\<int8_t\>      | int8*    | true  |
| plg::vector\<int16_t\>     | int16*   | true  |
| plg::vector\<int32_t\>     | int32*   | true  |
| plg::vector\<int64_t\>     | int64*   | true  |
| plg::vector\<uint8_t\>     | uint8*   | true  |
| plg::vector\<uint16_t\>    | uint16*  | true  |
| plg::vector\<uint32_t\>    | uint32*  | true  |
| plg::vector\<uint64_t\>    | uint64*  | true  |
| plg::vector\<uintptr_t\>   | ptr64*   | true  |
| plg::vector\<uintptr_t\>   | ptr32*   | true  |
| plg::vector\<float\>       | float*   | true  |
| plg::vector\<double\>      | double*  | true  |
| plg::vector\<plg::string\> | string*  | true  |
| plg::vec2                  | vec2     | true  |
| plg::vec3                  | vec3     | true  |
| plg::vec4                  | vec4     | true  |
| plg::mat4x4                | mat4x4   | true  |

## Exported Functions

### Example 1

- **Function Name:** Example_Function
- **Exported Method Name:** Example_Function_Exported_Name
- **Parameters:**
    - Parameter 1:
        - **Type:** ptr64
        - **Name:** param1
    - Parameter 2:
        - **Type:** string
        - **Name:** param2
    - Parameter 3:
        - **Type:** int32
        - **Name:** param3
    - Parameter 4:
        - **Type:** arrayint8
        - **Name:** param4
        - **Reference:** true
- **Return Type:** string

Here's an example template that combines these elements:

```json
{
  "name": "Example_Function",
  "funcName": "Example_Function_Exported_Name",
  "paramTypes": [
    {
      "type": "ptr64",
      "name": "param1"
    },
    {
      "type": "string",
      "name": "param2"
    },
    {
      "type": "int32",
      "name": "param3"
    },
    {
      "type": "arrayint8",
      "name": "param4",
      "ref": true
    }
  ],
  "retType": {
    "type": "string"
  }
}
```

How it will look like on C++ side:

```c++
extern "C" plg::string Example_Function(void* p1, const plg::string& p2, int32_t p3, plg::vector<uint8_t>& p4)
```

### Example 2

- **Function Name:** Example_Function
- **Exported Method Name:** Example_Function_Exported_Name
- **Parameters:**
    - Parameter 1:
        - **Type:** float
        - **Name:** param1
    - Parameter 2:
        - **Type:** double
        - **Name:** param2
        - **Reference:** true
    - Parameter 3:
        - **Type:** function
        - **Name:** param3
- **Return Type:** void

Here's an example template that combines these elements:

```json
{
  "name": "Example_Function",
  "funcName": "Example_Function_Exported_Name",
  "paramTypes": [
    {
      "type": "float",
      "name": "param1"
    },
    {
      "type": "double",
      "name": "param2",
      "ref": "true"
    },
    {
      "type": "function",
      "name": "param3",
      "prototype":
      {
        "name": "Example_Callback_Function",
        "funcName": "Example_Callback_Function_Exported_Name",
        "paramTypes": [
          {
            "type": "ptr64",
            "name": "param1"
          },
          {
            "type": "string",
            "name": "param2"
          },
          {
            "type": "int32",
            "name": "param3"
          }
        ],
        "retType": {
          "type": "string"
        }
      }
    }
  ],
  "retType": {
    "type": "void"
  }
}
```
