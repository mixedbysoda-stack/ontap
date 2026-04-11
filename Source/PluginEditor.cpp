#include "PluginEditor.h"

TapOnEditor::TapOnEditor(TapOnProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      mainPanel(p.getAPVTS())
#ifndef ONTAP_DEMO
      , activationDialog(p.getLicenseManager())
#endif
{
    setLookAndFeel(&tapOnLAF);
    addAndMakeVisible(mainPanel);

#ifndef ONTAP_DEMO
    addChildComponent(activationDialog);

    if (!processorRef.getLicenseManager().isActivated())
        activationDialog.setVisible(true);

    startTimerHz(4); // Check license state periodically
#endif

    setSize(880, 520);
    setResizable(true, false);
}

TapOnEditor::~TapOnEditor()
{
    setLookAndFeel(nullptr);
}

void TapOnEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0d0a1a));
}

void TapOnEditor::resized()
{
    mainPanel.setBounds(getLocalBounds());

#ifndef ONTAP_DEMO
    activationDialog.setBounds(getLocalBounds());
#endif
}

void TapOnEditor::timerCallback()
{
#ifndef ONTAP_DEMO
    if (processorRef.getLicenseManager().isActivated() && activationDialog.isVisible())
    {
        activationDialog.setVisible(false);
        repaint();
    }
#endif
}
