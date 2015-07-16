//******************************************************************************
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//******************************************************************************

#include "pch.h"
#include "FacebookError.h"

using namespace Platform;
using namespace Facebook;
using namespace Windows::Data::Json;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

FBError::FBError(
    ) :
    m_message(nullptr),
    m_type(nullptr),
    m_code(0),
    m_subcode(0),
    m_errorUserTitle(nullptr),
    m_errorUserMessage(nullptr)
{
    ;
}

String^ FBError::Message::get()
{
    return m_message;
}

String^ FBError::Type::get()
{
    return m_type;
}

int FBError::Code::get()
{
    return m_code;
}

int FBError::Subcode::get()
{
    return m_subcode;
}

String^ FBError::ErrorUserTitle::get()
{
    return m_errorUserTitle;
}

String^ FBError::ErrorUserMessage::get()
{
    return m_errorUserMessage;
}

FBError^ FBError::FromUri(
    Uri^ ResponseUri
    )
{
    FBError^ err = nullptr;
    bool foundCode = false;
    bool foundDescription = false;
    bool foundMessage = false;
    bool foundReason = false;
    int code = 0;
    String^ reason = nullptr;
    String^ description = nullptr;
    String^ message = nullptr;
    String^ query = ResponseUri->Query;

    if (query && !query->IsEmpty())
    {
        WwwFormUrlDecoder^ decoder = ref new WwwFormUrlDecoder(ResponseUri->Query);

        for (unsigned int i = 0; i < decoder->Size; i++)
        {
            IWwwFormUrlDecoderEntry^ entry = decoder->GetAt(i);
            if (entry->Name->Equals("error_code"))
            {
                foundCode = true;
                code = _wtoi(entry->Value->Data());
            }
            else if (entry->Name->Equals(L"error_description"))
            {
                foundDescription = true;
                description = entry->Value;
            }
            else if (entry->Name->Equals(L"error_message"))
            {
                foundMessage = true;
                message = entry->Value;
            }
            else if (entry->Name->Equals(L"error_reason"))
            {
                foundReason = true;
                reason = entry->Value;
            }
        }

        if (foundCode || foundDescription || foundMessage || foundReason)
        {
            err = ref new FBError();
            err->m_code = code;
            err->m_type = reason;
            if (foundDescription)
            {
                err->m_message = description;
            }
            else
            {
                err->m_message = message;
            }
        }
    }

    return err;
}

FBError^ FBError::FromJson(
    String^ JsonText
    )
{
    FBError^ result = nullptr;
    JsonValue^ val = nullptr;
    if (JsonValue::TryParse(JsonText, &val))
    {
        JsonObject^ obj = val->GetObject();
        IIterator<IKeyValuePair<String^, IJsonValue^>^>^ it = obj->First();
        String^ key = it->Current->Key;

        if (!String::CompareOrdinal(key, L"error"))
        {
            obj = it->Current->Value->GetObject();
        }

        result = ref new FBError();
        int found = 0;
        for (it = obj->First(); it->HasCurrent; it->MoveNext())
        {
            key = it->Current->Key;
            if (!String::CompareOrdinal(key, L"message"))
            {
                found++;
                result->m_message = it->Current->Value->GetString();
            }
            else if (!String::CompareOrdinal(key, L"type"))
            {
                found++;
                result->m_type = it->Current->Value->GetString();
            }
            else if (!String::CompareOrdinal(key, L"code"))
            {
                found++;
                result->m_code = static_cast<int>(
                    it->Current->Value->GetNumber());
            }
            else if (!String::CompareOrdinal(key, L"error_subcode"))
            {
                found++;
                result->m_subcode = static_cast<int>(
                    it->Current->Value->GetNumber());
            }
            else if (!String::CompareOrdinal(key, L"error_user_title"))
            {
                found++;
                result->m_errorUserTitle = it->Current->Value->GetString();
            }
            else if (!String::CompareOrdinal(key, L"error_user_msg"))
            {
                found++;
                result->m_errorUserMessage = it->Current->Value->GetString();
            }
        }

        // If we haven't recognized any fields, this isn't an error object
        if (!found)
        {
            result = nullptr;
        }
    }

    return result;
}

FBError::FBError(
    int Code,
    String^ Type,
    String^ Message
    ) :
    m_code(Code),
    m_errorUserMessage(nullptr),
    m_errorUserTitle(nullptr),
    m_message(Message),
    m_subcode(0),
    m_type(Type)
{
    ;
}
