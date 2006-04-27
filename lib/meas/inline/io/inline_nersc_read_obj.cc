// $Id: inline_nersc_read_obj.cc,v 3.1 2006-04-27 02:35:38 edwards Exp $
/*! \file
 * \brief Inline task to read an object from a named buffer
 *
 * Named object writing
 */

#include "chromabase.h"
#include "qdp_iogauge.h"
#include "meas/inline/abs_inline_measurement_factory.h"
#include "meas/inline/io/inline_nersc_read_obj.h"
#include "meas/inline/io/named_objmap.h"

namespace Chroma 
{ 
  namespace InlineNERSCReadNamedObjEnv 
  { 
    AbsInlineMeasurement* createMeasurement(XMLReader& xml_in, 
					    const std::string& path) 
    {
      return new InlineNERSCReadNamedObj(InlineNERSCReadNamedObjParams(xml_in, path));
    }

    const std::string name = "NERSC_READ_NAMED_OBJECT";

    bool registerAll() 
    {
      bool success = true; 
      success &= TheInlineMeasurementFactory::Instance().registerObject(name, createMeasurement);
      return success;
    }

    const bool registered = registerAll();
  };


  //! Object buffer
  void write(XMLWriter& xml, const string& path, const InlineNERSCReadNamedObjParams::NamedObject_t& input)
  {
    push(xml, path);

    write(xml, "object_id", input.object_id);

    pop(xml);
  }

  //! File output
  void write(XMLWriter& xml, const string& path, const InlineNERSCReadNamedObjParams::File_t& input)
  {
    push(xml, path);

    write(xml, "file_name", input.file_name);

    pop(xml);
  }


  //! Object buffer
  void read(XMLReader& xml, const string& path, InlineNERSCReadNamedObjParams::NamedObject_t& input)
  {
    XMLReader inputtop(xml, path);

    read(inputtop, "object_id", input.object_id);
  }

  //! File output
  void read(XMLReader& xml, const string& path, InlineNERSCReadNamedObjParams::File_t& input)
  {
    XMLReader inputtop(xml, path);

    read(inputtop, "file_name", input.file_name);
  }


  // Param stuff
  InlineNERSCReadNamedObjParams::InlineNERSCReadNamedObjParams() { frequency = 0; }

  InlineNERSCReadNamedObjParams::InlineNERSCReadNamedObjParams(XMLReader& xml_in, const std::string& path) 
  {
    try 
    {
      XMLReader paramtop(xml_in, path);

      if (paramtop.count("Frequency") == 1)
	read(paramtop, "Frequency", frequency);
      else
	frequency = 1;

      // Parameters for source construction
      read(paramtop, "NamedObject", named_obj);

      // Read in the destination
      read(paramtop, "File", file);
    }
    catch(const std::string& e) 
    {
      QDPIO::cerr << __func__ << ": caught Exception reading XML: " << e << endl;
      QDP_abort(1);
    }
  }


  void
  InlineNERSCReadNamedObjParams::write(XMLWriter& xml_out, const std::string& path) 
  {
    push(xml_out, path);
    
    // Parameters for source construction
    Chroma::write(xml_out, "NamedObject", named_obj);

    // Write out the destination
    Chroma::write(xml_out, "File", file);

    pop(xml_out);
  }


  void 
  InlineNERSCReadNamedObj::operator()(unsigned long update_no,
				       XMLWriter& xml_out) 
  {
    START_CODE();

    push(xml_out, "nersc_read_named_obj");
    write(xml_out, "update_no", update_no);

    QDPIO::cout << InlineNERSCReadNamedObjEnv::name << ": object reader" << endl;
    StopWatch swatch;

    // Read the object
    // ONLY SciDAC output format is supported in this task
    // Other tasks could support other disk formats
    QDPIO::cout << "Attempt to read object name = " << params.named_obj.object_id << endl;
    write(xml_out, "object_id", params.named_obj.object_id);
    try
    {
      swatch.reset();

      XMLReader record_xml;
      multi1d<LatticeColorMatrix> u(Nd);

      // Read the object
      swatch.start();
      QDP::readArchiv(record_xml, u, params.file.file_name);
      swatch.stop();

      XMLBufferWriter file_xml;
      push(file_xml, "NERSC");
      pop(file_xml);

      TheNamedObjMap::Instance().create< multi1d<LatticeColorMatrix> >(params.named_obj.object_id);
      TheNamedObjMap::Instance().getData< multi1d<LatticeColorMatrix> >(params.named_obj.object_id) = u;
      TheNamedObjMap::Instance().get(params.named_obj.object_id).setFileXML(file_xml);
      TheNamedObjMap::Instance().get(params.named_obj.object_id).setRecordXML(record_xml);

      QDPIO::cout << "Object successfully written: time= " 
		  << swatch.getTimeInSeconds() 
		  << " secs" << endl;
    }
    catch( std::bad_cast ) 
    {
      QDPIO::cerr << InlineNERSCReadNamedObjEnv::name << ": cast error" 
		  << endl;
      QDP_abort(1);
    }
    catch (const string& e) 
    {
      QDPIO::cerr << InlineNERSCReadNamedObjEnv::name << ": error message: " << e 
		  << endl;
      QDP_abort(1);
    }
    
    QDPIO::cout << InlineNERSCReadNamedObjEnv::name << ": ran successfully" << endl;

    pop(xml_out);  // read_named_obj

    END_CODE();
  } 

};
