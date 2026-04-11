import React from 'react'
interface LegoStudProps {
  active?: boolean
  color?: string
  activeColor?: string
  onClick?: () => void
  label?: string
  size?: 'sm' | 'md' | 'lg'
  round?: boolean
  className?: string
}
export function LegoStud({ active = false, color = '#A0A0A0', activeColor, onClick, label, size = 'md', round = true, className = '' }: LegoStudProps) {
  const resolvedColor = active && activeColor ? activeColor : color
  const sizeMap = { sm: { w: 20, h: 20, fontSize: '8px', studSize: 10 }, md: { w: 32, h: 32, fontSize: '10px', studSize: 16 }, lg: { w: 44, h: 44, fontSize: '12px', studSize: 22 } }
  const s = sizeMap[size]
  return (
    <button onClick={onClick} className={`flex flex-col items-center gap-1 group ${className}`} style={{ outline: 'none' }}>
      <div style={{ width: s.w, height: s.h, backgroundColor: resolvedColor, borderRadius: round ? '50%' : '3px', transform: active ? 'translateY(2px)' : 'translateY(0)', boxShadow: active ? 'inset 0 1px 2px rgba(255,255,255,0.2), inset 0 -1px 1px rgba(0,0,0,0.3), 0 1px 2px rgba(0,0,0,0.4)' : 'inset 0 2px 3px rgba(255,255,255,0.3), inset 0 -1px 2px rgba(0,0,0,0.2), 0 3px 6px rgba(0,0,0,0.5), 0 1px 2px rgba(0,0,0,0.3)', transition: 'all 0.1s ease', cursor: 'pointer', display: 'flex', alignItems: 'center', justifyContent: 'center', position: 'relative' }}>
        {round && <div style={{ width: s.studSize, height: s.studSize, borderRadius: '50%', border: '1px solid rgba(255,255,255,0.15)', boxShadow: 'inset 0 1px 2px rgba(0,0,0,0.2)' }} />}
      </div>
      {label && <span className="font-inter font-bold uppercase text-white text-center leading-none" style={{ fontSize: s.fontSize, letterSpacing: '0.02em' }}>{label}</span>}
    </button>
  )
}
