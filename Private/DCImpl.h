#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>

#include <pugixml/pugixml.hpp>

#include <Tera/DC.h>

namespace S1Data
{
  class DCExporterImpl {
  public:
    DCExporterImpl(DCInterface* storage)
      : Storage(storage)
    {}

    // x86

    void CreateXmlNode(DCElement86* element, pugi::xml_node& parent)
    {
      if (element && element->Name.Index)
      {
        pugi::xml_node node = parent.append_child(W2A(std::wstring(Storage->GetName(element->Name))).c_str());
        for (int32 idx = 0; idx < element->ChildrenCount; ++idx)
        {
          CreateXmlNode(Storage->GetElement86(element->ChildrenIndices, idx), node);
        }
        CreateXmlAttributes(element, node);
      }
    }

    void CreateXmlAttributes(DCElement86* element, pugi::xml_node& node)
    {
      if (element && element->Name.Index)
      {
        for (int32 idx = 0; idx < element->AttributeCount; ++idx)
        {
          DCAttribute86* attr = Storage->GetAttribute86(element->AttributeIndices, idx);
          if (!attr->Name.Index)
          {
            continue;
          }
          switch (attr->Type & 3)
          {
          case DCAttributeType::AT_Int:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            xattr.set_value(attr->Value.IntValue);
            break;
          }
          case DCAttributeType::AT_Float:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            xattr.set_value(attr->Value.FloatValue);
            break;
          }
          case DCAttributeType::AT_String:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            std::string tmp = W2A(Storage->GetString(attr->Value.IndexValue));
            xattr.set_value(tmp.c_str());
            break;
          }
          }
        }
      }
    }

    void CreateJsonNode(DCElement86* element, rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer)
    {
      writer->StartObject();
      if (element->AttributeCount)
      {
        for (int32 idx = 0; idx < element->AttributeCount; ++idx)
        {
          DCAttribute86* attr = Storage->GetAttribute86(element->AttributeIndices, idx);
          if (!attr->Name.Index || !attr->Type)
          {
            continue;
          }
          writer->Key(W2A(Storage->GetName(attr->Name)).c_str());
          switch (attr->Type & 3)
          {
          case DCAttributeType::AT_Int:
          {
            writer->Int(attr->Value.IntValue);
            break;
          }
          case DCAttributeType::AT_Float:
          {
            writer->Double(attr->Value.FloatValue);
            break;
          }
          case DCAttributeType::AT_String:
          {
            writer->String(W2A(Storage->GetString(attr->Value.IndexValue)).c_str());
            break;
          }
          }
        }
      }
      if (element->ChildrenCount)
      {
        bool foundName = false;
        for (int32 idx = 0; idx < element->ChildrenCount; ++idx)
        {
          if (DCElement86* child = Storage->GetElement86(element->ChildrenIndices, idx))
          {
            writer->String(W2A(Storage->GetName(child->Name)).c_str());
            foundName = true;
            break;
          }
        }
        if (!foundName)
        {
          writer->String(W2A(Storage->GetName(element->Name)).c_str());
        }
        writer->StartArray();
        for (int32 idx = 0; idx < element->ChildrenCount; ++idx)
        {
          CreateJsonNode(Storage->GetElement86(element->ChildrenIndices, idx), writer);
        }
        writer->EndArray();
      }
      writer->EndObject();
    }

    // x64

    void CreateXmlNode(DCElement64* element, pugi::xml_node& parent)
    {
      if (element && element->Name.Index)
      {
        pugi::xml_node node = parent.append_child(W2A(std::wstring(Storage->GetName(element->Name))).c_str());
        for (int32 idx = 0; idx < element->ChildrenCount; ++idx)
        {
          CreateXmlNode(Storage->GetElement64(element->ChildrenIndices, idx), node);
        }
        CreateXmlAttributes(element, node);
      }
    }

    void CreateXmlAttributes(DCElement64* element, pugi::xml_node& node)
    {
      if (element && element->Name.Index)
      {
        for (int32 idx = 0; idx < element->AttributeCount; ++idx)
        {
          DCAttribute64* attr = Storage->GetAttribute64(element->AttributeIndices, idx);
          if (!attr->Name.Index)
          {
            continue;
          }
          switch (attr->Type & 3)
          {
          case DCAttributeType::AT_Int:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            xattr.set_value(attr->Value.IntValue);
            break;
          }
          case DCAttributeType::AT_Float:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            xattr.set_value(attr->Value.FloatValue);
            break;
          }
          case DCAttributeType::AT_String:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            std::string tmp = W2A(Storage->GetString(attr->Value.IndexValue));
            xattr.set_value(tmp.c_str());
            break;
          }
          }
        }
      }
    }

    void CreateJsonNode(DCElement64* element, rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer)
    {
      writer->StartObject();
      if (element->AttributeCount)
      {
        for (int32 idx = 0; idx < element->AttributeCount; ++idx)
        {
          DCAttribute64* attr = Storage->GetAttribute64(element->AttributeIndices, idx);
          if (!attr->Name.Index || !attr->Type)
          {
            continue;
          }
          writer->Key(W2A(Storage->GetName(attr->Name)).c_str());
          switch (attr->Type & 3)
          {
          case DCAttributeType::AT_Int:
          {
            writer->Int(attr->Value.IntValue);
            break;
          }
          case DCAttributeType::AT_Float:
          {
            writer->Double(attr->Value.FloatValue);
            break;
          }
          case DCAttributeType::AT_String:
          {
            writer->String(W2A(Storage->GetString(attr->Value.IndexValue)).c_str());
            break;
          }
          }
        }
      }
      if (element->ChildrenCount)
      {
        bool foundName = false;
        for (int32 idx = 0; idx < element->ChildrenCount; ++idx)
        {
          if (DCElement64* child = Storage->GetElement64(element->ChildrenIndices, idx))
          {
            writer->String(W2A(Storage->GetName(child->Name)).c_str());
            foundName = true;
            break;
          }
        }
        if (!foundName)
        {
          writer->String(W2A(Storage->GetName(element->Name)).c_str());
        }
        writer->StartArray();
        for (int32 idx = 0; idx < element->ChildrenCount; ++idx)
        {
          CreateJsonNode(Storage->GetElement64(element->ChildrenIndices, idx), writer);
        }
        writer->EndArray();
      }
      writer->EndObject();
    }

  protected:
    DCInterface* Storage = nullptr;
  };
}