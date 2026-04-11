import React, { useCallback, useState, useRef } from 'react'
interface LegoKnobProps {
  value: number
  onChange: (value: number) => void
  label?: string
  size?: number
  color?: string
  readout?: string
  showArc?: boolean
  arcColor?: string
}
export function LegoKnob({ value, onChange, label, size = 72, color = '#333333', readout, showArc = false, arcColor = '#ff6b2b' }: LegoKnobProps) {
  const [isDragging, setIsDragging] = useState(false)
  const dragStartY = useRef(0)
  const dragStartValue = useRef(0)
  const rotation = (value / 100) * 270 - 135
  const handleMouseDown = useCallback((e: React.MouseEvent) => {
    e.preventDefault()
    setIsDragging(true)
    dragStartY.current = e.clientY
    dragStartValue.current = value
    const handleMouseMove = (e: MouseEvent) => {
      const delta = (dragStartY.current - e.clientY) * 0.5
      const newValue = Math.max(0, Math.min(100, dragStartValue.current + delta))
      onChange(newValue)
    }
    const handleMouseUp = () => {
      setIsDragging(false)
      window.removeEventListener('mousemove', handleMouseMove)
      window.removeEventListener('mouseup', handleMouseUp)
      document.body.style.cursor = ''
    }
    document.body.style.cursor = 'ns-resize'
    window.addEventListener('mousemove', handleMouseMove)
    window.addEventListener('mouseup', handleMouseUp)
  }, [value, onChange])

  const arcPadding = 8
  const arcRadius = size / 2 + arcPadding
  const arcStroke = 4
  const svgSize = (arcRadius + arcStroke) * 2
  const center = svgSize / 2
  const startAngle = 135
  const totalSweep = 270
  const valueAngle = startAngle + (value / 100) * totalSweep

  function polarToCartesian(cx: number, cy: number, r: number, angleDeg: number) {
    const rad = ((angleDeg - 90) * Math.PI) / 180
    return { x: cx + r * Math.cos(rad), y: cy + r * Math.sin(rad) }
  }
  function describeArc(cx: number, cy: number, r: number, start: number, end: number) {
    const s = polarToCartesian(cx, cy, r, start)
    const e = polarToCartesian(cx, cy, r, end)
    const sweep = end - start
    const largeArc = sweep > 180 ? 1 : 0
    return `M ${s.x} ${s.y} A ${r} ${r} 0 ${largeArc} 1 ${e.x} ${e.y}`
  }

  const innerSize = size * 0.78
  const studSize = size * 0.16

  return (
    <div className="flex flex-col items-center gap-1">
      {label && <span className="font-inter font-bold uppercase tracking-wide" style={{ fontSize: size > 100 ? 14 : 10, color: 'rgba(255,255,255,0.5)' }}>{label}</span>}
      <div style={{ position: 'relative', width: svgSize, height: svgSize, display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
        {showArc && (
          <svg width={svgSize} height={svgSize} style={{ position: 'absolute', top: 0, left: 0 }}>
            <path d={describeArc(center, center, arcRadius, startAngle, startAngle + totalSweep)} fill="none" stroke="rgba(255,255,255,0.08)" strokeWidth={arcStroke} strokeLinecap="round" />
            {value > 0.5 && <path d={describeArc(center, center, arcRadius, startAngle, valueAngle)} fill="none" stroke={arcColor} strokeWidth={arcStroke + 1} strokeLinecap="round" style={{ filter: `drop-shadow(0 0 4px ${arcColor}60)` }} />}
          </svg>
        )}
        <div onMouseDown={handleMouseDown} style={{ width: size, height: size, borderRadius: '50%', backgroundColor: color, cursor: isDragging ? 'ns-resize' : 'pointer', boxShadow: 'inset 0 2px 6px rgba(255,255,255,0.12), inset 0 -3px 6px rgba(0,0,0,0.4), 0 4px 12px rgba(0,0,0,0.6), 0 1px 3px rgba(0,0,0,0.4)', display: 'flex', alignItems: 'center', justifyContent: 'center', position: 'relative', userSelect: 'none' }}>
          <div style={{ width: size * 0.88, height: size * 0.88, borderRadius: '50%', border: '1px solid rgba(255,255,255,0.06)', position: 'absolute' }} />
          <div style={{ width: innerSize, height: innerSize, borderRadius: '50%', backgroundColor: color, border: '2px solid rgba(255,255,255,0.05)', boxShadow: 'inset 0 2px 4px rgba(0,0,0,0.4), inset 0 -1px 3px rgba(255,255,255,0.08)', display: 'flex', alignItems: 'center', justifyContent: 'center', transform: `rotate(${rotation}deg)`, transition: isDragging ? 'none' : 'transform 0.1s ease', position: 'relative' }}>
            <div style={{ width: studSize * 1.8, height: studSize * 1.8, borderRadius: '50%', backgroundColor: 'rgba(255,255,255,0.06)', boxShadow: 'inset 0 1px 3px rgba(0,0,0,0.3)', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
              <div style={{ width: studSize, height: studSize, borderRadius: '50%', backgroundColor: 'rgba(255,255,255,0.1)', boxShadow: 'inset 0 1px 2px rgba(0,0,0,0.3), 0 1px 1px rgba(255,255,255,0.05)' }} />
            </div>
            <div style={{ width: studSize * 0.7, height: studSize * 0.7, borderRadius: '50%', backgroundColor: arcColor, boxShadow: `0 0 6px ${arcColor}80`, position: 'absolute', top: size * 0.08, left: '50%', transform: 'translateX(-50%)' }} />
          </div>
        </div>
      </div>
      {readout && <span className="font-inter font-bold text-white tabular-nums" style={{ fontSize: size > 100 ? 16 : 11 }}>{readout}</span>}
    </div>
  )
}
