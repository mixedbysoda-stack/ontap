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

void TapOnEditor::parentHierarchyChanged()
{
    juce::AudioProcessorEditor::parentHierarchyChanged();

   #if JUCE_WINDOWS
    // Force JUCE 8's GDI software renderer instead of the new Direct2D backend.
    // Direct2D context init crashes on Intel Iris Xe integrated graphics inside
    // sandboxed plug-in host processes (Bitwig PluginHost, Reaper plug-in
    // scanner, MuLab) — same class of crash that took out Pour's demo on Win 11
    // Iris Xe rigs. Software renderer adds negligible CPU cost and is rock-solid
    // across GPU configs. Applied preemptively to On Tap so its Windows demo
    // never hits the same wall.
    if (auto* peer = getPeer())
    {
        const auto engines = peer->getAvailableRenderingEngines();
        const int softIdx = engines.indexOf("Software Renderer");
        if (softIdx >= 0 && peer->getCurrentRenderingEngine() != softIdx)
            peer->setCurrentRenderingEngine(softIdx);
    }
   #endif
}
