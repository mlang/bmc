// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include <iostream>

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

class document
{
  XERCES_CPP_NAMESPACE::DOMDocument *dom_document;
  XERCES_CPP_NAMESPACE::DOMElement *score_partwise, *part_list;
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
      xml_string dtd_public("-//Recordare//DTD MusicXML 2.0 Partwise//EN");
      xml_string dtd_url("http://www.musicxml.org/dtds/partwise.dtd");

      dom_document = dom->createDocument
	(nullptr, type, dom->createDocumentType(type, dtd_public, dtd_url));

      score_partwise = dom_document->getDocumentElement();
      score_partwise->setAttribute(xml_string("version"), xml_string("2.0"));
      DOMElement* identification = dom_document->createElement(xml_string("identification"));
      DOMElement* encoding = dom_document->createElement(xml_string("encoding"));
      DOMElement* software = dom_document->createElement(xml_string("software"));
      DOMText*    text = dom_document->createTextNode(xml_string("Braille Music Compiler"));
      software->appendChild(text);
      encoding->appendChild(software);
      identification->appendChild(encoding);
      score_partwise->appendChild(identification);

      part_list = dom_document->createElement(xml_string("part-list"));
      score_partwise->appendChild(part_list);

      return true;
    }
    return false;
  }

public:
  bool to_stdout() const
  {
    XERCES_CPP_NAMESPACE_USE

    try {
      DOMImplementationLS *ls =
	DOMImplementationRegistry::getDOMImplementation(xml_string("ls"));
      DOMLSSerializer *serializer = ls->createLSSerializer();
      DOMConfiguration *configuration = serializer->getDomConfig();

      if (configuration->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
	configuration->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);

      DOMLSOutput *output = ls->createLSOutput();
      StdOutFormatTarget format_target;
      output->setByteStream(&format_target);
      serializer->write(dom_document, output);

      output->release();
      serializer->release();

      return true;
    }
    catch (const OutOfMemoryException&) {
      std::cerr << "OutOfMemoryException" << std::endl;
    } catch (XMLException& e) {
      std::cerr << "An error occurred during creation of output transcoder. Msg is:"
		<< std::endl
		<< e.getMessage() << std::endl;
    }
    return false;
  }

  ~document() { if (dom_document) dom_document->release(); }
};

}}

#include <xercesc/util/PlatformUtils.hpp>

int main()
{
  XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize();
  {
    music::musicxml::document musicxml;
    musicxml.to_stdout();
  }
  XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate();
  return EXIT_SUCCESS;
}
