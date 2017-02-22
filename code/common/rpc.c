/* Copyright (c) 2014,2015 kacangbawang.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// ------------------------ INCLUDES ---------------------------------------- //
#include "common.h"
#include <stdio.h>  //for printf, snprintf
#include <string.h> //for strncmp, strlen

#include "jsmn.h"
#include "rpc.h"

#define assert(x)

// -------------------------- PRINTING ------------------------------------- //
// Define your `printf` implementation (printf, uart_printf, ...)
// extern uart_context_t* g_psDBGUARTCtx;
//#define DEBUG_PRINTF(...)	UARTprintf(g_psDBGUARTCtx, __VA_ARGS__)
#define DEBUG_PRINTF(...)

// ------------------------ GLOBALS ----------------------------------------- //
// This code is primarily meant for microcontrollers, so use of malloc and
// friends
// is purpusefully avoided.

// -- all tokens
#define TOKENS_MAXSIZE 32
static jsmntok_t g_psTokens[TOKENS_MAXSIZE]; // 8 bytes each, 256 bytes total
static int g_iNumTokens;

// -- jsonrpc elements as tokens
static jsmntok_t *g_psTokVersion;
static jsmntok_t *g_psTokMethod;
static jsmntok_t *g_psTokParams;
static jsmntok_t *g_psTokId;

static const methodtable_entry_t *g_psMethodTable;
static unsigned int g_uiNumMethods;

static const char JRPC_JSONRPC_KEY[] = "jsonrpc";
static const char JRPC_JSONRPC_VALUE[] = "2.0";
static const char JRPC_METHOD_KEY[] = "method";
static const char JRPC_PARAMS_KEY[] = "params";
static const char JRPC_ID_KEY[] = "id";
static const char JRPC_NULL[] = "null";

#define JSON_MATCHED(String, Token, Buffer)                                    \
  (strlen(String) == (size_t)((Token)->end - (Token)->start) &&                \
   strncmp(&(Buffer[(Token)->start]), String, strlen(String)) == 0)

#define JSON_IS_NULL(String, Token)                                            \
  ((Token)->type == JSMN_PRIMITIVE && JSON_MATCHED(String, Token, JRPC_NULL))

// ------------------------ FUNCTIONS --------------------------------------- //

// -------------------------------------------------------------------------- //
//
// Saves a pointer to a table of methods, which is passed in from the outside
// but only if all checks pass. Otherwise do nothing.
//
// INPUT:   array of methods to be called via RPC (this should be a static
// object)
// EFFECTS: saves a pointer to the table, and number of entries
// OUTPUT:  status code
//
// -------------------------------------------------------------------------- //
workstatus_t rpc_install_methods(const methodtable_entry_t *psMethods,
                                 unsigned int uiNumMethods) {

  for (unsigned int i = 0; i < uiNumMethods; i++) {
    // check name to be non-empty
    if (psMethods[i].name == NULL || psMethods[i].name[0] == 0) {
      return WORKSTATUS_RPC_ERROR_INSTALLMETHODS;
    }

    // check sig format
    if (psMethods[i].sig == NULL) {
      return WORKSTATUS_RPC_ERROR_INSTALLMETHODS;
    }

    // min length:3, first char: (, last 2 chars: ){PSAO}
    unsigned int uiSigLen = strlen(psMethods[i].sig);
    if (uiSigLen < 3 || psMethods[i].sig[0] != '(' ||
        psMethods[i].sig[uiSigLen - 2] != ')') {
      return WORKSTATUS_RPC_ERROR_INSTALLMETHODS;
    }

    switch (psMethods[i].sig[uiSigLen - 1]) {
    case 'P':
    case 'S':
    case 'A':
    case 'O':
      break;
    default:
      return WORKSTATUS_RPC_ERROR_INSTALLMETHODS;
    }

    // check that callback are defined for all of the method
    if (psMethods[i].func == NULL) {
      return WORKSTATUS_RPC_ERROR_INSTALLMETHODS;
    }
  }

  g_psMethodTable = psMethods;
  g_uiNumMethods = uiNumMethods;
  return WORKSTATUS_NO_ERROR;
}

// -------------------------------------------------------------------------- //
//
// Performs parsing on the json and checks for parsing-related errors only
//
// INPUT:   command string, which contains exactly one line (\n terminated)
// EFFECTS: loads the static token array
// OUTPUT:  status code
//
// TODO: iCommandLen should be a unsigned int, but for that uart_console needs
// to be fixed first
// -------------------------------------------------------------------------- //
static workstatus_t rpc_parse_command(const char *const pcCommand,
                                      int iCommandLen) {

  int iRes;
  jsmn_parser sParser;

  jsmn_init(&sParser);
  iRes = jsmn_parse(&sParser, pcCommand, iCommandLen, g_psTokens,
                    sizeof(g_psTokens) / sizeof(g_psTokens[0]));

  // if error during parse, return translated code
  if (iRes < 0) {
    switch (iRes) {
    case JSMN_ERROR_INVAL:
      return WORKSTATUS_PARSE_ERROR_INVAL;
    case JSMN_ERROR_NOMEM:
      return WORKSTATUS_PARSE_ERROR_NOMEM;
    case JSMN_ERROR_PART:
      return WORKSTATUS_PARSE_ERROR_PART;
    default:
      assert(0);
    }
  }

  // ** DEBUG **
  //    if (iRes > 0) {
  //        depth_first_dump(pcCommand, g_psTokens, 0, 0);
  //    }

  // total number of tokens found
  g_iNumTokens = iRes;
  return WORKSTATUS_NO_ERROR;
}

// -------------------------------------------------------------------------- //
//
// Here we check the RPC requirements (JSON-RPC Version 2)
//
// INPUT:   command string, which contains exactly one line (\n terminated)
// EFFECTS: loads static RPC-related tokens
// OUTPUT:  status code
//
// -------------------------------------------------------------------------- //
static workstatus_t rpc_validate_rpc(const char *const pcCommand) {
  // if we're here, we know that object parsed correctly and we can start
  // looking at the tokens

  // go over top level, and find these tokens
  g_psTokVersion = NULL;
  g_psTokMethod = NULL;
  g_psTokParams = NULL;
  g_psTokId = NULL;

  bool bBadParams = false;
  bool bBadId = false;

  // we require at least one object, that being the outer {}
  if (g_iNumTokens < 1 || g_psTokens[0].type != JSMN_OBJECT) {
    return WORKSTATUS_RPC_ERROR_INVALIDOUTER;
  }

  // find tokens of interest, and do some simple checks on them
  if (g_psTokens[0].size > 0) {
    int sibling = g_psTokens[0].first_child;
    do {
      if (JSON_MATCHED(JRPC_JSONRPC_KEY, &g_psTokens[sibling], pcCommand)) {
        g_psTokVersion = &g_psTokens[sibling];
        if (g_psTokVersion->size != 1 ||
            g_psTokens[g_psTokVersion->first_child].type != JSMN_STRING ||
            !JSON_MATCHED(JRPC_JSONRPC_VALUE,
                          &g_psTokens[g_psTokVersion->first_child],
                          pcCommand)) {
          // we changed our mind, we don't want it
          g_psTokVersion = NULL;
        }
      } else if (JSON_MATCHED(JRPC_METHOD_KEY, &g_psTokens[sibling],
                              pcCommand)) {
        g_psTokMethod = &g_psTokens[sibling];
        if (g_psTokMethod->size != 1 ||
            g_psTokens[g_psTokMethod->first_child].type != JSMN_STRING) {
          g_psTokMethod = NULL;
        }
      } else if (JSON_MATCHED(JRPC_PARAMS_KEY, &g_psTokens[sibling],
                              pcCommand)) {
        g_psTokParams = &g_psTokens[sibling];
        //... must be object or array
        if (g_psTokParams->size != 1 ||
            (g_psTokens[g_psTokParams->first_child].type != JSMN_OBJECT &&
             g_psTokens[g_psTokParams->first_child].type != JSMN_ARRAY)) {
          bBadParams = true;
          g_psTokParams = NULL;
        }
      } else if (JSON_MATCHED(JRPC_ID_KEY, &g_psTokens[sibling], pcCommand)) {
        g_psTokId = &g_psTokens[sibling];
        //... must be STRING, NUMBER or NULL
        // TODO: Add additional checks to weed out NON NUMBER and NON NULL.
        //      Currently a primitive is just some unquoted ascii that starts
        //      with a certain letter
        if (g_psTokId->size != 1 ||
            (g_psTokens[g_psTokId->first_child].type != JSMN_STRING &&
             g_psTokens[g_psTokId->first_child].type != JSMN_PRIMITIVE)) {
          bBadId = true;
          g_psTokId = NULL;
        }
      }
    } while ((sibling = g_psTokens[sibling].next_sibling) != -1);
  }

  // check version. must be present, and "just so"
  if (g_psTokVersion == NULL) {
    return WORKSTATUS_RPC_ERROR_INVALIDVERSION;
  }

  // check method. must be present
  if (g_psTokMethod == NULL) {
    return WORKSTATUS_RPC_ERROR_INVALIDMETHOD;
  }

  // check params. If present...
  //    if (g_psTokParams == NULL) {
  //        return WORKSTATUS_RPC_ERROR_INVALIDPARAMS;
  //    }
  // params are optional, but if they're there, we check for validity
  if (bBadParams) {
    return WORKSTATUS_RPC_ERROR_INVALIDPARAMS;
  }

  // check id. If present...
  //    if (g_psTokId == NULL) {
  //        return WORKSTATUS_RPC_ERROR_INVALIDID;
  //    }
  // id is optional, but if it's there, we check for validity
  if (bBadId) {
    return WORKSTATUS_RPC_ERROR_INVALIDID;
  }

  return WORKSTATUS_NO_ERROR;
}

// -------------------------------------------------------------------------- //
//
// Tries to find desired method in method table by name. Nothing else.
//
// INPUT:   command string, which contains exactly one line (\n terminated)
// EFFECTS: none
// OUTPUT:  status code
//
// -------------------------------------------------------------------------- //
static workstatus_t rpc_validate_method(const char *const pcCommand,
                                        int *piMethod) {
  if (g_psMethodTable && g_psTokMethod && (g_psTokMethod->size == 1)) {
    for (unsigned int i = 0; i < g_uiNumMethods; i++) {
      if (JSON_MATCHED(g_psMethodTable[i].name,
                       &g_psTokens[g_psTokMethod->first_child], pcCommand)) {
        if (piMethod) {
          *piMethod = i;
        }
        return WORKSTATUS_NO_ERROR;
      }
    }
  }

  if (piMethod) {
    *piMethod = -1;
  }
  return WORKSTATUS_RPC_ERROR_METHODNOTFOUND;
}

//

// -------------------------------------------------------------------------- //
//
//  We need to make sure the total number, types, and order matches,
//  however, we make no judgement beyond the broad JSMN type
//  ie, JSMN_PRIMITIVE could be 3,true,null or nullwithjunk.
//  All these are valid PRIMITIVEs as far as JSMN is concerned.
//  And that is all that we guarantee in this method.
//  The rpc method itself will need to conduct a more detailed check (if
//  desired).
//  Pay special attention to the case of NO PARAMS
//
//  parameter interpretation:
//  []      size:0 start:n end:n+2
//  [""]    size: 1
//  ""      size: 0, start: n, end: n
//
//  Signature string:
//  -----------------
//  This is very much like JNI, but with only 4 types.
//
//  (params)return_value
//  P = primitive, S = string, A = array, O = object
//  Only the params are checked here, not return type
//  For void, use P. For NULL use P.
//
//  Example:
//  (PS)P = primitive function(primitive p, string s)
//  ()P = primitive function(void)
//
// INPUT:   command string (\n terminated), index of method for which we're
//          checking parameters
// EFFECTS: none
// OUTPUT:  status code
//
// -------------------------------------------------------------------------- //

static workstatus_t rpc_validate_params(const char *const pcCommand,
                                        int iMethod) {

  // safety check
  if (!pcCommand || iMethod == -1) {
    return WORKSTATUS_RPC_ERROR_PARAMSMISMATCH;
  }

  // no params? (null params token, params token of size 0,
  //            first child of params of size 0 or less)
  if (g_psTokParams == NULL || g_psTokParams->size == 0 ||
      g_psTokens[g_psTokParams->first_child].size <= 0) {
    if (strlen(g_psMethodTable[iMethod].sig) - 3 != 0) {
      return WORKSTATUS_RPC_ERROR_PARAMSMISMATCH;
    }
    return WORKSTATUS_NO_ERROR;
  }

  // length check
  if (strlen(g_psMethodTable[iMethod].sig) - 3 /*()X*/
      != g_psTokens[g_psTokParams->first_child].size) {
    return WORKSTATUS_RPC_ERROR_PARAMSMISMATCH;
  }

  // type check. if we're here, there has to be at least one element
  // in the params array
  unsigned int iSigCounter = 1;
  int iSibling = g_psTokens[g_psTokParams->first_child].first_child;
  do {
    switch (g_psTokens[iSibling].type) {
    case JSMN_PRIMITIVE:
      if ((g_psMethodTable[iMethod].sig)[iSigCounter++] != 'P') {
        return WORKSTATUS_RPC_ERROR_PARAMSMISMATCH;
      }
      break;
    case JSMN_STRING:
      if ((g_psMethodTable[iMethod].sig)[iSigCounter++] != 'S') {
        return WORKSTATUS_RPC_ERROR_PARAMSMISMATCH;
      }
      break;
    case JSMN_ARRAY:
      if ((g_psMethodTable[iMethod].sig)[iSigCounter++] != 'A') {
        return WORKSTATUS_RPC_ERROR_PARAMSMISMATCH;
      }
      break;
    case JSMN_OBJECT:
      if ((g_psMethodTable[iMethod].sig)[iSigCounter++] != 'O') {
        return WORKSTATUS_RPC_ERROR_PARAMSMISMATCH;
      }
      break;
    default:
      assert(0);
    }
  } while ((iSibling = g_psTokens[iSibling].next_sibling) != -1);

  return WORKSTATUS_NO_ERROR;
}

// -------------------------------------------------------------------------- //
//
// Forms the bulk of the JSON response string and calls the specified method
// If no error was reported from method, closes the JSON response and returns.
//
// INPUT:   command string, method index, response array, response array size
// EFFECTS: calls the function given in the method
// OUTPUT:  status code
//
// -------------------------------------------------------------------------- //
static workstatus_t rpc_call_method(const char *const pcCommand, int iMethod,
                                    char *pcResponseBuffer,
                                    int iResponseBufferLen) {

  assert(g_psMethodTable[iMethod].func);

  // if no response buffer or no id - no need bother with all this return value
  // stuff
  if (!pcResponseBuffer || iResponseBufferLen < 1 || !g_psTokId ||
      g_psTokId->size != 1) {
    return g_psMethodTable[iMethod].func(pcCommand, g_psTokens, g_psTokParams,
                                         NULL, 0);
  }

  // prep string (will be null terminated)
  int iTotalLen = snprintf(
      pcResponseBuffer, iResponseBufferLen,
      "{\"jsonrpc\":\"2.0\", \"id\":%s%.*s%s, \"result\":",
      g_psTokens[g_psTokId->first_child].type == JSMN_STRING ? "\"" : "",
      g_psTokens[g_psTokId->first_child].end -
          g_psTokens[g_psTokId->first_child].start,
      &pcCommand[g_psTokens[g_psTokId->first_child].start],
      g_psTokens[g_psTokId->first_child].type == JSMN_STRING ? "\"" : "");

  // check that there is at least one more empty spot for '}' (zero already
  // there)
  if (iTotalLen + 2 /*plus },0*/ > iResponseBufferLen) {
    pcResponseBuffer[0] = 0;
    return WORKSTATUS_RPC_ERROR_OUTOFRESBUF;
  }

  // call method
  // UARTRPC_PRINTF("method will start writing at: %d, max chars: %d\n",
  // iTotalLen, iRespMaxLen-iTotalLen-2);
  workstatus_t eRet = g_psMethodTable[iMethod].func(
      pcCommand, g_psTokParams, g_psTokens, pcResponseBuffer + iTotalLen,
      iResponseBufferLen - iTotalLen - 2); //-2 for },0

  // if method returned error, dont' bother with json string, it will get
  // overwritten
  if (eRet != WORKSTATUS_NO_ERROR) {
    return eRet;
  }

  // close the curly bracket (string will be null terminated again)
  // don't know how many chars function wrote, so need to measure
  iTotalLen = strlen(pcResponseBuffer);
  if (iTotalLen + 3 <=
      iResponseBufferLen) { // 3 for },\n,\0              //2 for },\0
    pcResponseBuffer[iTotalLen] = '}';
    pcResponseBuffer[iTotalLen + 1] = '\n';
    pcResponseBuffer[iTotalLen + 2] = '\0';
  }

  return eRet;
}

// -------------------------------------------------------------------------- //
//
// Helper function to print JSONRPC error responses
//
// INPUT:   command string, response buffer, response buffer size, error status
// code
// EFFECTS: fills the response buffer with JSON error object, up to the buffer
// size
// OUTPUT:  total number of character that *would have been* printed into the
// response buffer
//          (this may be more than it can handle, and this condition is checked
//          in the caller)
//
// -------------------------------------------------------------------------- //
static int rpc_print_error_json(const char *pcCommand, char *pcResponse,
                                int iRespMaxLen, workstatus_t eStatus) {
  int iTotalLen = snprintf(
      pcResponse, iRespMaxLen, "{\"jsonrpc\":\"2.0\", \"error\":{\"code\":%d, "
                               "\"message\":\"%s\"}, \"id\":%s%.*s%s}\n",
      eStatus, workstatus_to_string(eStatus),
      g_psTokens[g_psTokId->first_child].type == JSMN_STRING ? "\"" : "",
      g_psTokens[g_psTokId->first_child].end -
          g_psTokens[g_psTokId->first_child].start,
      &pcCommand[g_psTokens[g_psTokId->first_child].start],
      g_psTokens[g_psTokId->first_child].type == JSMN_STRING ? "\"" : "");
  return iTotalLen;
}

// -------------------------------------------------------------------------- //
//
// This is the workhorse function for handling RPC requests. It takes in a
// string
// and "returns" a string, without knowing the specifics of the method by which
// the string is being sent/received. In addition to the string, it provides
// detailed error codes describing which step in the RPC sequence triggered it.
//
// This method goes through a number of validation steps (parsing, rpc
// specifics,
// method and params, and finally calls the method. The method table must be
// set ahead of time via the 'install_methods' function.
//
// This function also takes care of forming the JSONRPC response, whether it be
// error or success. The returned error code does not indicate whether the
// response
// buffer should be sent back (there are many possibilities when that is true).
// The length of the response buffer indicates whether it is meant to be sent
// back. Zero-length: NO, non-zero length: YES.
//
// Currently the response JSON is not validated. Nor is the function return
// type checked.
//
// INPUT:   command string, command string len, response buffer, response buf
// len
// EFFECTS: sets global tokens, executes requested method, clears global tokens,
//          fills response buffer (see output)
// OUTPUT:  returns: status code (for caller's logic. This is a detailed status
//          code) and response buffer (uses simplified status codes,
//          as per JSONRPC specs. This is returned to the remote caller)
//
// -------------------------------------------------------------------------- //
workstatus_t rpc_handle_command(const char *const pcCommand, int iCommandLen,
                                char *pcResponse, int iRespMaxLen) {
  workstatus_t eStatus = WORKSTATUS_NO_ERROR;
  int iMethod = -1;

  eStatus = rpc_parse_command(pcCommand, iCommandLen);
  if (eStatus != WORKSTATUS_NO_ERROR) {
    goto L_done;
  }

  eStatus = rpc_validate_rpc(pcCommand);
  if (eStatus != WORKSTATUS_NO_ERROR) {
    goto L_done;
  }

  eStatus = rpc_validate_method(pcCommand, &iMethod);
  if (eStatus != WORKSTATUS_NO_ERROR) {
    goto L_done;
  }

  eStatus = rpc_validate_params(pcCommand, iMethod);
  if (eStatus != WORKSTATUS_NO_ERROR) {
    goto L_done;
  }

  eStatus = rpc_call_method(pcCommand, iMethod, pcResponse, iRespMaxLen);
  if (eStatus != WORKSTATUS_NO_ERROR) {
    goto L_done;
  }

// TODO: validate fuction-returned json?
// We know that there is no error triggered so far
// We know that the RPC function has already done its payload... ***
// - overall json parsing check
// - rpc response requirements
// - return type

L_done:
  // form json response
  if (!pcResponse || iRespMaxLen <= 0 || !g_psTokId ||
      JSON_IS_NULL(pcCommand, &g_psTokens[g_psTokId->first_child])) {
    // no response
    if (pcResponse && iRespMaxLen > 0) {
      pcResponse[0] = 0;
    }
  } else {
    // means pcResponse exists, id token exists, id token is not 'null'
    int iTotalLen = 0;
    switch (eStatus) {
    case WORKSTATUS_NO_ERROR:
      // response already formed by function
      break;
    // parse error
    case WORKSTATUS_PARSE_ERROR_NOMEM:
    case WORKSTATUS_PARSE_ERROR_INVAL:
    case WORKSTATUS_PARSE_ERROR_PART:
      iTotalLen = rpc_print_error_json(pcCommand, pcResponse, iRespMaxLen,
                                       JSONRPC_20_PARSE_ERROR);
      break;
    // request malformed
    case WORKSTATUS_RPC_ERROR_INVALIDOUTER:
    case WORKSTATUS_RPC_ERROR_INVALIDVERSION:
    case WORKSTATUS_RPC_ERROR_INVALIDID:
    case WORKSTATUS_RPC_ERROR_INVALIDMETHOD:
    case WORKSTATUS_RPC_ERROR_INVALIDPARAMS:
      iTotalLen = rpc_print_error_json(pcCommand, pcResponse, iRespMaxLen,
                                       JSONRPC_20_INVALID_REQUEST);
      break;
    // params mismatch
    case WORKSTATUS_RPC_ERROR_PARAMSMISMATCH:
      iTotalLen = rpc_print_error_json(pcCommand, pcResponse, iRespMaxLen,
                                       JSONRPC_20_INVALIDPARAMS);
      break;
    // method not found
    case WORKSTATUS_RPC_ERROR_METHODNOTFOUND:
      iTotalLen = rpc_print_error_json(pcCommand, pcResponse, iRespMaxLen,
                                       JSONRPC_20_METHODNOTFOUND);
      break;
    // internal
    case WORKSTATUS_RPC_ERROR_INSTALLMETHODS:
    case WORKSTATUS_RPC_ERROR_OUTOFRESBUF:
      iTotalLen = rpc_print_error_json(pcCommand, pcResponse, iRespMaxLen,
                                       JSONRPC_20_INTERNALERROR);
      break;
    default:
      assert(0);
    }

    // check snprintf retval, return no response (would be misformated anyway)
    // plus a special return code
    if (iTotalLen > iRespMaxLen) {
      pcResponse[0] = 0;
      eStatus = WORKSTATUS_RPC_ERROR_OUTOFRESBUF;
    }

    // TODO: other checks on reply format here?
    // - json format
    // - rpc format
    // - method return value
    // this is the second place where an error check can be located
    // however we're checking an error message for errors... recursion
    // follows???
  }

  // reset for the next run
  g_psTokVersion = NULL;
  g_psTokId = NULL;
  g_psTokMethod = NULL;
  g_psTokParams = NULL;
  g_iNumTokens = 0;

  return eStatus;
}

// -------------------------------------------------------------------------- //
//
// Provides a human readable explanation of error code
//
// INPUT:   status enum
// EFFECTS: none
// OUTPUT:  returns a const string containing an explanation of the status code
//
// -------------------------------------------------------------------------- //
const char *workstatus_to_string(workstatus_t eStatus) {
  switch (eStatus) {
  case WORKSTATUS_NO_ERROR:
    return "WORKSTATUS_NO_ERROR: no error";
  case WORKSTATUS_PARSE_ERROR_NOMEM:
    return "WORKSTATUS_PARSE_ERROR_NOMEM: not enough tokens available";
  case WORKSTATUS_PARSE_ERROR_INVAL:
    return "WORKSTATUS_PARSE_ERROR_INVAL: invalid json character encountered";
  case WORKSTATUS_PARSE_ERROR_PART:
    return "WORKSTATUS_PARSE_ERROR_PART: json string not terminated";
  case WORKSTATUS_RPC_ERROR_INVALIDOUTER:
    return "WORKSTATUS_PARSE_ERROR_NOTOBJECT: outer json layer is not an "
           "object";
  case WORKSTATUS_RPC_ERROR_INVALIDVERSION:
    return "WORKSTATUS_RPC_ERROR_INVALIDVERSION: Version string must be "
           "present and equal to 2.0";
  case WORKSTATUS_RPC_ERROR_INVALIDID:
    return "WORKSTATUS_RPC_ERROR_INVALIDID: Id, if present, must be "
           "string/number/null";
  case WORKSTATUS_RPC_ERROR_INVALIDMETHOD:
    return "WORKSTATUS_RPC_ERROR_INVALIDMETHOD: Method, must be present and "
           "must be a string";
  case WORKSTATUS_RPC_ERROR_INVALIDPARAMS:
    return "WORKSTATUS_RPC_ERROR_INVALIDPARAMS: Params, if present, must be "
           "array/object";
  case WORKSTATUS_RPC_ERROR_METHODNOTFOUND:
    return "WORKSTATUS_RPC_ERROR_METHODNOTDEFINED: no such method defined, or "
           "attempting to define NULL method";
  case WORKSTATUS_RPC_ERROR_PARAMSMISMATCH:
    return "WORKSTATUS_RPC_ERROR_PARAMSMISMATCH: params mismatch for requested "
           "method";
  case WORKSTATUS_RPC_ERROR_INSTALLMETHODS:
    return "WORKSTATUS_RPC_ERROR_METHODFORMAT: RPC method install failed, "
           "check name/sig/function prototype";
  case WORKSTATUS_RPC_ERROR_OUTOFRESBUF:
    return "WORKSTATUS_RPC_ERROR_PRINTRESPONSE: Ran out of buffer printing "
           "JSON response";

  /* These messages are purposefully short, as they will be sent over the wire
   */
  case JSONRPC_20_PARSE_ERROR:
    return "json parsing error";
  case JSONRPC_20_INVALID_REQUEST:
    return "json rpc structure error";
  case JSONRPC_20_METHODNOTFOUND:
    return "remote method not found";
  case JSONRPC_20_INVALIDPARAMS:
    return "wrong params for remote method";
  case JSONRPC_20_INTERNALERROR:
    return "internal error";

  default:
    assert(0);
  }
  return NULL;
}
