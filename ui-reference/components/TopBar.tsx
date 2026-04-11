import React from 'react'
import { RefreshCwIcon, MenuIcon } from 'lucide-react'
interface TopBarProps {
  rate: string
  onRateChange: (rate: string) => void
  trigger: string
  onTriggerChange: (trigger: string) => void
  bypassed: boolean
  onBypassToggle: () => void
}
const RATES = ['1/8', '1/4', '1/2', '1/1']
const TRIGGERS = ['SYNC', 'MIDI', 'AUDIO']
export function TopBar({ rate, onRateChange, trigger, onTriggerChange, bypassed, onBypassToggle }: TopBarProps) {
  return (
    <div className="flex items-center justify-between px-3 relative" style={{ height: 42, backgroundColor: '#1a1530', borderBottom: '1px solid rgba(255,255,255,0.06)' }}>
      <div className="absolute top-0 left-0 right-0" style={{ height: 4, backgroundImage: 'radial-gradient(circle, rgba(255,255,255,0.04) 3px, transparent 3px)', backgroundSize: '18px 4px', backgroundPosition: '9px 0', backgroundRepeat: 'repeat-x' }} />
      <div style={{ width: 280 }} />
      <div className="flex items-center gap-6">
        <div className="flex items-center gap-1">
          {RATES.map((r) => (
            <button key={r} onClick={() => onRateChange(r)} style={{ padding: '4px 14px', borderRadius: 14, backgroundColor: rate === r ? '#ff6b2b' : 'transparent', color: rate === r ? '#0d0a1a' : '#b0adc0', fontSize: 13, fontWeight: 700, fontFamily: 'Inter, sans-serif', cursor: 'pointer', border: 'none', outline: 'none', transition: 'all 0.15s ease', boxShadow: rate === r ? 'inset 0 1px 2px rgba(255,255,255,0.3), 0 2px 4px rgba(0,0,0,0.4)' : 'none' }}>{r}</button>
          ))}
        </div>
        <div className="flex items-center gap-1">
          {TRIGGERS.map((t) => (
            <button key={t} onClick={() => onTriggerChange(t)} style={{ padding: '4px 12px', borderRadius: 3, backgroundColor: 'transparent', color: trigger === t ? '#ffffff' : '#6b6880', fontSize: 12, fontWeight: 700, fontFamily: 'Inter, sans-serif', letterSpacing: '0.05em', cursor: 'pointer', border: trigger === t ? '1.5px solid rgba(255,255,255,0.5)' : '1.5px solid transparent', outline: 'none', transition: 'all 0.15s ease', boxShadow: trigger === t ? 'inset 0 1px 1px rgba(255,255,255,0.1), 0 1px 3px rgba(0,0,0,0.3)' : 'none' }}>{t}</button>
          ))}
        </div>
      </div>
      <div className="flex items-center gap-3">
        <button onClick={onBypassToggle} style={{ width: 28, height: 28, borderRadius: '50%', backgroundColor: bypassed ? 'rgba(255,107,43,0.15)' : 'transparent', border: 'none', outline: 'none', cursor: 'pointer', display: 'flex', alignItems: 'center', justifyContent: 'center', transition: 'all 0.15s ease' }}>
          <RefreshCwIcon size={16} color={bypassed ? '#ff6b2b' : '#6b6880'} />
        </button>
        <button style={{ width: 28, height: 28, borderRadius: 4, backgroundColor: 'transparent', border: 'none', outline: 'none', cursor: 'pointer', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
          <MenuIcon size={18} color="#6b6880" />
        </button>
      </div>
    </div>
  )
}
