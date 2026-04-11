import React, { useState } from 'react'
import { TopBar } from './components/TopBar'
import { LeftPanel } from './components/LeftPanel'
import { Visualizer } from './components/Visualizer'
export function App() {
  const [bypassed, setBypassed] = useState(false)
  const [mix, setMix] = useState(79)
  const [bandActive, setBandActive] = useState(false)
  const [frequency, setFrequency] = useState(50)
  const [activePreset, setActivePreset] = useState(4) // S-curve like reference
  const [rate, setRate] = useState('1/4')
  const [trigger, setTrigger] = useState('AUDIO')
  return (
    <div className="flex items-center justify-center w-full min-h-screen bg-[#0d0a1a]">
      <div
        className="relative flex flex-col overflow-hidden"
        style={{
          width: 1200,
          height: 600,
          minWidth: 1200,
          minHeight: 600,
          maxWidth: 1200,
          maxHeight: 600,
          borderRadius: 8,
          backgroundColor: '#151025',
          boxShadow: '0 12px 48px rgba(0,0,0,0.8), 0 2px 8px rgba(0,0,0,0.6)',
          opacity: bypassed ? 0.55 : 1,
          transition: 'opacity 0.3s ease',
          backgroundImage:
            'radial-gradient(circle, rgba(255,255,255,0.02) 1px, transparent 1px)',
          backgroundSize: '20px 20px',
          backgroundPosition: '10px 10px',
        }}
      >
        <TopBar
          rate={rate}
          onRateChange={setRate}
          trigger={trigger}
          onTriggerChange={setTrigger}
          bypassed={bypassed}
          onBypassToggle={() => setBypassed(!bypassed)}
        />
        <div className="flex flex-1 min-h-0">
          <LeftPanel
            mix={mix}
            onMixChange={setMix}
            bandActive={bandActive}
            onBandToggle={() => setBandActive(!bandActive)}
            frequency={frequency}
            onFrequencyChange={setFrequency}
          />
          <Visualizer
            activePreset={activePreset}
            onPresetChange={setActivePreset}
          />
        </div>
        {bypassed && (
          <div
            className="absolute inset-0 pointer-events-none flex items-center justify-center"
            style={{ zIndex: 50 }}
          >
            <span
              className="font-fredoka uppercase tracking-widest"
              style={{
                fontSize: 52,
                color: 'rgba(255,255,255,0.08)',
                transform: 'rotate(-15deg)',
                textShadow: '0 2px 8px rgba(0,0,0,0.5)',
              }}
            >
              BYPASSED
            </span>
          </div>
        )}
      </div>
    </div>
  )
}
