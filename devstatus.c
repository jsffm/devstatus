/*
 * devstatus.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#include <sys/ioctl.h>
#include <linux/dvb/frontend.h>

#include <vdr/plugin.h>
#include <vdr/status.h>
#include <vdr/menu.h>
#include "i18n.h"

static const char *VERSION        = "0.2.0";
static const char *DESCRIPTION    = trNOOP("Status of dvb devices");
static const char *MAINMENUENTRY  = trNOOP("Device status");

#undef DAYDATETIMESTRING
#if VDRVERSNUM >= 10318
#define DAYDATETIME(x) *DayDateTime(x)
#else
#define DAYDATETIME(x) DayDateTime(x)
#endif


static int showRecordings = 1;
static int showStrength  = 1;
static int showChannels  = 1;


const char* cardtypeAsString(int typ) {
   switch (typ) {
         case FE_QPSK:   return tr("satellite card");
         case FE_QAM:    return tr("cable card");    break;
         case FE_OFDM:   return tr("terrestrial card");    break;
         default:        return tr("unknown cardtype");
   };
}


// --- cRecObj --------------------------------------------------------
class cRecObj : public cListObject {
public:
    char* name;
    const cDevice* device;
    cTimer* timer;
public:
    cRecObj(const char* Name, const cDevice* Device, cTimer* Timer)
        {
            name = strdup(Name);
            device = Device;
            timer = Timer;
        }
    ~cRecObj()
        {
            if (name) free(name);
        }
 };

class cRecStatusMonitor : public cStatus 
{
protected:
#if VDRVERSNUM >= 10338
    virtual void Recording(const cDevice *Device, const char *Name, const char *FileName, bool On);
#else
    virtual void Recording(const cDevice *Device, const char *Name);
#endif
 public:
    cRecStatusMonitor();
};

cList<cRecObj> CurrentRecordings;

// --- cMenuRecItem ------------------------------------------------------
class cMenuRecItem : public cOsdItem {
  char *Name;
public: 
  int ChannelNr; 
  cMenuRecItem(const char* name) { 
          Name = NULL;
          ChannelNr = 0;
          if (name) {  Name = strdup(name); SetText(Name, false); }
  }
  cMenuRecItem(const cRecObj* r) { 
          Name = NULL;
          if (r->name) {  
              Name = strdup(r->name); 
              char* itemText = NULL;
              asprintf(&itemText, "%s\t%s", DAYDATETIME(r->timer->StartTime()), Name);
              SetText(itemText, false); 
          }
  }
  char* RecName()  { return Name; }
  int GetChannelNr()  { return ChannelNr; }
  bool IsChannel() { return ChannelNr != 0; }
};


class cMenuRecStatus : public cOsdMenu {
private:
public:
     void deviceinfoAsString(cDevice *d)
     {
        struct dvb_frontend_info m_FrontendInfo;
        int m_Frontend;
        fe_status_t status;
        uint16_t signal = 0;
        uint16_t snr = 0;
        uint32_t ber = 0;
        char* output = NULL;
        #define FRONTEND_DEVICE "/dev/dvb/adapter%d/frontend%d"

        cString dev = cString::sprintf(FRONTEND_DEVICE, d->CardIndex(), 0);
        m_Frontend = open(dev, O_RDONLY | O_NONBLOCK);
        if (m_Frontend < 0) {
           return;
        }
        CHECK(ioctl(m_Frontend, FE_GET_INFO, &m_FrontendInfo));
        CHECK(ioctl(m_Frontend, FE_READ_STATUS, &status));
        CHECK(ioctl(m_Frontend, FE_READ_SIGNAL_STRENGTH, &signal));
        CHECK(ioctl(m_Frontend, FE_READ_SNR, &snr));
        CHECK(ioctl(m_Frontend, FE_READ_BER, &ber));
        close(m_Frontend);

        asprintf(&output, "%s (%s) - /dev/dvb/adapter%d", 
                          cardtypeAsString(m_FrontendInfo.type), 
                          m_FrontendInfo.name,
                          d->CardIndex()
        );
        cMenuRecItem* norec =  new cMenuRecItem(output);
        norec->SetSelectable(false);
        Add(norec);
        free(output);

        if( showStrength ) {
            asprintf(&output, tr("signal: %d %%, s/n: %d %%"), signal / 655, snr / 655 );
            norec =  new cMenuRecItem(output);
            norec->SetSelectable(false);
            Add(norec);
            free(output);
        }
     }


    cMenuRecStatus():cOsdMenu(tr("Device status"), 15)
      {
          Write();
      }

    void Write (void)
      {
          Clear(); // clear OSD
          for (int i = 0; i < cDevice::NumDevices(); i++) 
          {
              cDevice *d = cDevice::GetDevice(i);
              char* devName = NULL;
              char* devInfo = NULL;
              if (d->HasDecoder() || d->IsPrimaryDevice())
                  asprintf(&devInfo, " (%s%s%s)", 
                    d->HasDecoder() ? tr("device with decoder") : "", 
                    (d->HasDecoder() && d->IsPrimaryDevice()) ? ", " : "", 
                    d->IsPrimaryDevice() ? tr("primary device") : ""
                  );
             
              asprintf(&devName, "--- %s %d %s %s ---",
                        tr("Device"), 
                        i+1, 
                        (i == cDevice::ActualDevice()->CardIndex()) ? tr("-- Live"):"",
                        devInfo ? devInfo : ""
              ) ;
              cMenuRecItem* DeviceHeader =  new cMenuRecItem(devName);
              DeviceHeader->SetSelectable(true);
              Add(DeviceHeader);
              free(devName);
              if (devInfo)
                  free(devInfo);

              deviceinfoAsString(d);

              if( showRecordings ) {
                  int Count = 0;
                  for (cRecObj *r = CurrentRecordings.First(); r; r = CurrentRecordings.Next(r)) {
                      if (r && r->device == d) {
                          Add(new cMenuRecItem(r));
                          Count++;
                      }
                  }
                  if (Count == 0) {
                      cMenuRecItem* norec =  new cMenuRecItem(tr("currently no recordings"));
                      norec->SetSelectable(false);
                      Add(norec);
                  }
              }

                     
              if (showChannels) {
                 cMenuRecItem* norec = NULL;
                 char* output = NULL;
                 int channelNo;
                 cChannel *channel = NULL;
       
                 for (channelNo = 1; channelNo <= Channels.MaxNumber(); channelNo++) {
                    if( (channel = Channels.GetByNumber(channelNo)) ) {
                       if (d->IsTunedToTransponder(channel)) {
                           if( channelNo == d->CurrentChannel()) 
                              asprintf(&output, tr("  tuned to %4d + %s"), channelNo, channel->Name() );
                           else 
                              asprintf(&output, tr("  tuned to %4d - %s"), channelNo, channel->Name() );
                           norec = new cMenuRecItem(output);
                           norec->ChannelNr = channelNo;
                           norec->SetSelectable(true);
                           Add(norec);
                           free(output);
                       }
                    }
                 }
              }

              // Leerzeile schreiben
              if (i < cDevice::NumDevices()) {
                  DeviceHeader =  new cMenuRecItem("");
                  DeviceHeader->SetSelectable(false);
                  Add(DeviceHeader);
              }
          }      

          cMenuRecItem* norec = NULL;

          norec = new cMenuRecItem("");
          norec->SetSelectable(true); // so we can scroll to the end of the list
          Add(norec);

          SetHelp( showRecordings ? tr("no recordings"):tr("recordings"), 
                   showStrength   ? tr("no strength")  :tr("strength"),
                   showChannels   ? tr("no channels")  :tr("channels"), 
                   tr("Refresh display")
                 );
          Display();       
          
      }

    eOSState Play(char* file)
        {
            cRecording* recordingFound = NULL;
            for(cRecording* recording = Recordings.First(); recording; recording = Recordings.Next(recording))
                if (strstr(recording->Title(), file))
                    recordingFound = recording;
            if (!recordingFound)
                return osContinue;
            cReplayControl::SetRecording(recordingFound->FileName(), recordingFound->Title());
            return osReplay;
        }

    eOSState ProcessKey(eKeys Key)
        {
            cMenuRecItem *ri;
            eOSState state = cOsdMenu::ProcessKey(Key);     
            if (state == osUnknown) 
            {
                switch(Key)
                {
                  case kRed:
                        showRecordings = !showRecordings;
                        Write();
                        break;
                  case kGreen:
                        showStrength = !showStrength;
                        Write();
                        break;
                  case kYellow:
                        showChannels = !showChannels;
                        Write();
                        break;
                  case kBlue:
                        Write();
                        break;
                  case kChanUp:
                  case kChanDn: 
                  case k7: 
                  case k9:
                        ri = (cMenuRecItem*)Get(Current());
                        if (ri->Selectable())
                            if (ri->IsChannel()) {
                                Channels.SwitchTo(ri->GetChannelNr() + ((Key==k9)||(Key==kChanUp)? 1:-1) );
                                Write(); //repaint; maybe 'Live' has changed
                                return osContinue;
                            }
                        break;
                       
                  case kOk: { 
                        ri = (cMenuRecItem*)Get(Current());
                        if (ri->Selectable())
                            if (ri->IsChannel()) {
                                Channels.SwitchTo(ri->GetChannelNr());
                                Write(); //repaint; maybe 'Live' has changed
                                return osContinue;
                            } else 
                                return Play(ri->RecName());
                        break;
                        }
                  default:
                    state = osContinue;
                    break;
                }
            }
            return state;
        }
  };



class cPluginRecstatus : public cPlugin {
private:
    cRecStatusMonitor* recStatusMonitor;
  // Add any member variables or functions you may need here.
public:
  cPluginRecstatus(void);
  virtual ~cPluginRecstatus();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return tr(DESCRIPTION); }
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual bool Start(void);
  virtual void Stop(void);
  virtual void Housekeeping(void);
  virtual const char *MainMenuEntry(void) { return tr(MAINMENUENTRY); }
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  virtual bool Service(const char *Id, void *Data = NULL);
  virtual const char **SVDRPHelpPages(void);
  virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
  };

cPluginRecstatus::cPluginRecstatus(void)
{
    recStatusMonitor = NULL;
}

cPluginRecstatus::~cPluginRecstatus()
{
    delete recStatusMonitor;
}

bool cPluginRecstatus::ProcessArgs(int argc, char *argv[])
{
  // Implement command line argument processing here if applicable.
  return true;
}

bool cPluginRecstatus::Initialize(void)
{
#if APIVERSNUM < 10507
   RegisterI18n(Phrases);
#endif
  return true;
}

bool cPluginRecstatus::Start(void)
{
    recStatusMonitor = new cRecStatusMonitor;
    return true;
}

void cPluginRecstatus::Stop(void)
{
  // Stop any background activities the plugin shall perform.
}

void cPluginRecstatus::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}

cOsdObject *cPluginRecstatus::MainMenuAction(void)
{
    return new cMenuRecStatus();
}

cMenuSetupPage *cPluginRecstatus::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return NULL;
}

bool cPluginRecstatus::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  return false;
}

bool cPluginRecstatus::Service(const char *Id, void *Data)
{
  // Handle custom service requests from other plugins
  return false;
}

const char **cPluginRecstatus::SVDRPHelpPages(void)
{
  // Return help text for SVDRP commands this plugin implements
  static const char *HelpPages[] = {
    "DEVSTAT\n"
    "    Print all devices with their Recordingstats.",
    "RECNUMBER\n"
    "    Print number of concurrent recordings for all devices.",
    NULL
    };
  return HelpPages;
}

cString cPluginRecstatus::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
  // Process SVDRP commands this plugin implements
  if(strcasecmp(Command, "RECSTAT") == 0) {
    char* output = NULL;
    asprintf(&output, "%s:\n", tr("List of DVB devices"));
    for (int i = 0; i < cDevice::NumDevices(); i++)
    {
      cDevice *d = cDevice::GetDevice(i);
      char* devName = NULL;
      char* devInfo = NULL;
      if (d->HasDecoder() || d->IsPrimaryDevice())
        asprintf(&devInfo, " (%s%s%s)", d->HasDecoder() ? tr("device with decoder") : "", (d->HasDecoder() && d->IsPrimaryDevice()) ? ", " : "", d->IsPrimaryDevice() ? tr("primary device") : "");
      asprintf(&devName, "--- %s %d%s ---", tr("Device"), i+1, devInfo ? devInfo : "");
      asprintf(&output, "%s  %s:\n", output , devName); // add device output and there info's
      free(devName);
      if (devInfo)
        free(devInfo);
    int Count = 0;
    for (cRecObj *r = CurrentRecordings.First(); r; r = CurrentRecordings.Next(r)){ // add recordings to the output
      if (r && r->device == d){
        char* Name = NULL;
        if (r->name){
          Name = strdup(r->name);
          char* itemText = NULL;
          asprintf(&itemText, "%s %s", DAYDATETIME(r->timer->StartTime()), Name);
          asprintf(&output, "%s    %s\n", output , itemText);
          free(itemText);
        }
        Count++;
      }
    }
    if (Count == 0)
      asprintf(&output, "%s    %s\n", output, tr("currently no recordings"));
    if (i < cDevice::NumDevices())
      asprintf(&output, "%s\n", output);
    }
    // we use the default reply code here
    return cString::sprintf("%s", output);
  }

  if(strcasecmp(Command, "RECNUMBER") == 0) {
    char* output = NULL;
    asprintf(&output, "%s:\n", tr("Number of concurrent recordings"));
    for (int i = 0; i < cDevice::NumDevices(); i++)
    {
      cDevice *d = cDevice::GetDevice(i);
      char* devName = NULL;
      asprintf(&devName, "%s %d", tr("Device"), i+1);
      asprintf(&output, "%s %s:", output , devName); // add device output and there info's
      free(devName);
    int Count = 0;
    for (cRecObj *r = CurrentRecordings.First(); r; r = CurrentRecordings.Next(r)){ // add recordings to the output
      if (r && r->device == d){
        Count++;
      }
    }
    asprintf(&output, "%s %i\n", output, Count);
    if (i < cDevice::NumDevices())
      asprintf(&output, "%s\n", output);
    }
    // we use the default reply code here
    return cString::sprintf("%s", output);
  }

  return NULL;
}


/*--------*/

cRecStatusMonitor::cRecStatusMonitor()
{
}


#if VDRVERSNUM >= 10338 
void cRecStatusMonitor::Recording(const cDevice *Device, const char *Name, const char *FileName, bool On)
#else
void cRecStatusMonitor::Recording(const cDevice *Device, const char *Name)
#endif
{
    // insert new timers currently recording in TimersRecording
    if (Name)
    {
        for (cTimer *ti = Timers.First(); ti; ti = Timers.Next(ti)) 
            if (ti->Recording())
            {
                // check if this is a new entry
                bool bFound = false;
                for (cRecObj *r = CurrentRecordings.First(); r; r = CurrentRecordings.Next(r)) 
                    if (r->timer == ti)
                        bFound = true;

                if (bFound) continue; // already handled
                CurrentRecordings.Add(new cRecObj(Name, Device, ti));
                return;
            }
    }
    
    if (!Name)
    {
        // remove timers that finished recording from TimersRecording
        for (cRecObj *r = CurrentRecordings.First(); r; r = CurrentRecordings.Next(r)) 
        {
            if (!r->timer->Recording())
            {
                CurrentRecordings.Del(r);
                break;
            }
        }
    }
}



             

VDRPLUGINCREATOR(cPluginRecstatus); // Don't touch this!
