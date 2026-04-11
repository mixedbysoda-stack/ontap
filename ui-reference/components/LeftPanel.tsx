import React from 'react'
import { LegoKnob } from './LegoKnob'
import { HeadphonesIcon } from 'lucide-react'
interface LeftPanelProps {
  mix: number
  onMixChange: (v: number) => void
  bandActive: boolean
  onBandToggle: () => void
  frequency: number
  onFrequencyChange: (v: number) => void
}
function freqToDisplay(value: number): string {
  const minLog = Math.log10(20)
  const maxLog = Math.log10(20000)
  const freq = Math.pow(10, minLog + (value / 100) * (maxLog - minLog))
  if (freq >= 1000) return `${(freq / 1000).toFixed(1)}kHz`
  return `${Math.round(freq)}Hz`
}
export function LeftPanel({
  mix, onMixChange, bandActive, onBandToggle, frequency, onFrequencyChange,
}: LeftPanelProps) {
  return (
    <div className="flex flex-col relative" style={{ width: 280, minWidth: 280, backgroundColor: '#151025', padding: '14px 20px 12px', borderRight: '1px solid rgba(255,255,255,0.06)' }}>
      <span className="font-inter font-bold uppercase tracking-widest" style={{ fontSize: 9, color: '#6b6880', letterSpacing: '0.2em' }}>CARBONATED AUDIO</span>
      <h1 className="font-fredoka tracking-wide" style={{ fontSize: 28, color: '#ffffff', lineHeight: 1.1, marginTop: 2, letterSpacing: '1px' }}>ON TAP</h1>
      <div className="flex-1 flex items-center justify-center" style={{ marginTop: -4 }}>
        <LegoKnob value={mix} onChange={onMixChange} size={160} color="#1a1530" showArc arcColor="#ff6b2b" readout={`MIX ${Math.round(mix)}%`} />
      </div>
      <div className="flex flex-col gap-2">
        <div className="flex items-center gap-2">
          <button onClick={onBandToggle} style={{ padding: '5px 16px', borderRadius: 14, backgroundColor: bandActive ? '#ff6b2b' : 'transparent', color: bandActive ? '#0d0a1a' : '#b0adc0', fontSize: 11, fontWeight: 800, fontFamily: 'Inter, sans-serif', letterSpacing: '0.08em', cursor: 'pointer', border: bandActive ? 'none' : '1.5px solid rgba(255,255,255,0.15)', outline: 'none', transition: 'all 0.15s ease', boxShadow: bandActive ? 'inset 0 1px 2px rgba(255,255,255,0.3), 0 2px 4px rgba(0,0,0,0.4)' : 'none' }}>BAND</button>
          <HeadphonesIcon size={16} color="#6b6880" style={{ marginLeft: 2 }} />
        </div>
        <div style={{ opacity: bandActive ? 1 : 0.3, transition: 'opacity 0.25s ease', pointerEvents: bandActive ? 'auto' : 'none' }}>
          <div className="relative" style={{ width: '100%', height: 10, backgroundColor: '#0d0a1a', borderRadius: 5, boxShadow: 'inset 0 1px 3px rgba(0,0,0,0.6)', cursor: 'pointer' }}
            onMouseDown={(e) => {
              e.preventDefault()
              const bar = e.currentTarget
              const rect = bar.getBoundingClientRect()
              const update = (clientX: number) => { const pct = Math.max(0, Math.min(100, ((clientX - rect.left) / rect.width) * 100)); onFrequencyChange(pct) }
              update(e.clientX)
              const handleMove = (ev: MouseEvent) => update(ev.clientX)
              const handleUp = () => { window.removeEventListener('mousemove', handleMove); window.removeEventListener('mouseup', handleUp); document.body.style.cursor = '' }
              document.body.style.cursor = 'ew-resize'
              window.addEventListener('mousemove', handleMove)
              window.addEventListener('mouseup', handleUp)
            }}>
            <div style={{ width: `${frequency}%`, height: '100%', backgroundColor: '#ff6b2b', borderRadius: 5, boxShadow: 'inset 0 1px 2px rgba(255,255,255,0.3), 0 0 6px rgba(255,107,43,0.3)', transition: 'width 0.05s ease' }} />
          </div>
          {bandActive && (
            <div className="flex justify-between mt-1">
              <span className="font-inter font-bold" style={{ fontSize: 8, color: '#6b6880' }}>20Hz</span>
              <span className="font-inter font-bold tabular-nums" style={{ fontSize: 9, color: '#ff6b2b' }}>{freqToDisplay(frequency)}</span>
              <span className="font-inter font-bold" style={{ fontSize: 8, color: '#6b6880' }}>20kHz</span>
            </div>
          )}
        </div>
      </div>
    </div>
  )
}
