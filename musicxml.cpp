// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include <ctime>
#include <iostream>
#include <memory>

namespace music { namespace musicxml {

class xml_string
{
  XMLCh *transcoded;
public:
  xml_string(char const *const string)
  : transcoded(XERCES_CPP_NAMESPACE::XMLString::transcode(string))
  {}
  ~xml_string() { XERCES_CPP_NAMESPACE::XMLString::release(&transcoded); }

  operator XMLCh const *() const { return transcoded; }
};

class ostream_format_target: public XERCES_CPP_NAMESPACE::XMLFormatTarget
{
  std::ostream& stream;
public:
  ostream_format_target(std::ostream &stream): stream(stream) {}

  virtual void writeChars( const XMLByte* const buffer, const XMLSize_t size
                         , xercesc::XMLFormatter* const
                         )
  {
    if (not stream.fail())
      stream.write(reinterpret_cast<const char*>(buffer),
                   static_cast<std::streamsize>(size));
  }

  virtual void flush() { if (not stream.fail()) stream.flush(); }
};

struct dom_deleter
{
  template <class T>
  void operator()(T* ptr) { ptr->release(); }
};

class document
{
  std::unique_ptr<xercesc::DOMDocument, dom_deleter> dom_document;
  xercesc::DOMElement *score_partwise, *part_list;
public:
  document() { create_empty_document(); }

private:
  bool create_empty_document()
  {
    XERCES_CPP_NAMESPACE_USE

    DOMImplementation *dom =
      DOMImplementationRegistry::getDOMImplementation(xml_string("Core"));
    if (dom) {
      xml_string type("score-partwise");
      xml_string dtd_public("-//Recordare//DTD MusicXML 3.0 Partwise//EN");
      xml_string dtd_url("http://www.musicxml.org/dtds/partwise.dtd");

      dom_document.reset
      (dom->createDocument
       (nullptr, type, dom->createDocumentType(type, dtd_public, dtd_url)));

      score_partwise = dom_document->getDocumentElement();
      score_partwise->setAttribute(xml_string("version"), xml_string("3.0"));
      DOMElement* identification =
	dom_document->createElement(xml_string("identification"));
      DOMElement* encoding = dom_document->createElement(xml_string("encoding"));
      encoding->appendChild(encoding_date());
      DOMElement* software = dom_document->createElement(xml_string("software"));
      software->appendChild(dom_document->createTextNode(xml_string("Braille Music Compiler")));
      encoding->appendChild(software);
      identification->appendChild(encoding);
      score_partwise->appendChild(identification);

      part_list = dom_document->createElement(xml_string("part-list"));
      score_partwise->appendChild(part_list);

      return true;
    }
    return false;
  }

  xercesc::DOMElement *encoding_date() const
  {
    xercesc::DOMElement *element = dom_document->createElement(xml_string("encoding-date"));
    char date_string[11];
    time_t t1 = std::time(NULL);
    struct tm *t2 = std::localtime(&t1);
    std::strftime(date_string, sizeof(date_string), "%Y-%m-%d", t2);
    element->appendChild(dom_document->createTextNode(xml_string(date_string)));
    return element;
  }

public:
  bool write_to_stream(std::ostream& stream) const
  {
    XERCES_CPP_NAMESPACE_USE

    try {
      DOMImplementationLS *ls =
	DOMImplementationRegistry::getDOMImplementation(xml_string("ls"));
      std::unique_ptr<DOMLSSerializer, dom_deleter>
      serializer(ls->createLSSerializer());
      DOMConfiguration *configuration = serializer->getDomConfig();

      if (configuration->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
	configuration->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);

      std::unique_ptr<DOMLSOutput, dom_deleter> output(ls->createLSOutput());
      ostream_format_target format_target(stream);
      output->setByteStream(&format_target);

      return serializer->write(dom_document.get(), output.get());
    } catch (const OutOfMemoryException&) {
      std::cerr << "OutOfMemoryException" << std::endl;
    } catch (XMLException& e) {
      std::cerr << "An error occurred during creation of output transcoder. Msg is:"
		<< std::endl
		<< e.getMessage() << std::endl;
    }
    return false;
  }
};

}}

#include <xercesc/util/PlatformUtils.hpp>

int main()
{
  XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize();
  {
    music::musicxml::document musicxml;
    musicxml.write_to_stream(std::cout);
  }
  XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate();
  return EXIT_SUCCESS;
}
