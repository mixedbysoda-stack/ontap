import React, { useCallback, useEffect, useRef } from 'react'
import { ChevronLeftIcon, ChevronRightIcon, ChevronsLeftIcon, ChevronsRightIcon } from 'lucide-react'

interface VisualizerProps {
  activePreset: number
  onPresetChange: (index: number) => void
}

export const CURVE_PRESETS: ((x: number) => number)[] = [
  (x) => (x < 0.05 ? 1 : Math.exp(-x * 8) * 0.9),
  (x) => Math.exp(-x * 4),
  (x) => Math.exp(-x * 2),
  (x) => 1 - x,
  (x) => 1 - 1 / (1 + Math.exp(-12 * (x - 0.5))),
  (x) => (x < 0.1 ? 1 - x * 5 : 0.5 * Math.exp(-(x - 0.1) * 6)),
  (x) => (x < 0.5 ? Math.exp(-x * 6) : 0),
  (x) => (1 + Math.cos(x * Math.PI)) / 2,
  (x) => Math.abs(Math.sin(x * Math.PI * 2)) * Math.exp(-x * 3),
  (x) => x,
  (x) => (x < 0.33 ? 1 : x < 0.66 ? 0.5 : 0),
  (x) => Math.exp(-x * 1.5),
  (x) => (x < 0.02 ? 1 : Math.exp(-x * 16) * 0.7),
  (x) => Math.exp(-x * 3) * (0.5 + 0.5 * Math.cos(x * Math.PI * 6)),
  (x) => (x < 0.3 ? 1 : Math.exp(-(x - 0.3) * 5)),
  (x) => (x < 0.15 ? 1 - x * 6 : Math.max(0, 0.1 - (x - 0.15) * 0.15)),
]

function MiniPreview({ curveFn, active, onClick }: { curveFn: (x: number) => number; active: boolean; onClick: () => void }) {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return
    const ctx = canvas.getContext('2d')
    if (!ctx) return
    const w = canvas.width, h = canvas.height
    ctx.clearRect(0, 0, w, h)
    ctx.strokeStyle = active ? '#ff6b2b' : 'rgba(255,255,255,0.25)'
    ctx.lineWidth = active ? 2 : 1.5
    ctx.beginPath()
    for (let i = 0; i <= w; i++) {
      const x = i / w
      const y = h - curveFn(x) * (h - 4) - 2
      if (i === 0) ctx.moveTo(i, y); else ctx.lineTo(i, y)
    }
    ctx.stroke()
  }, [curveFn, active])
  return (
    <button onClick={onClick} style={{ width: 78, height: 48, borderRadius: 4, backgroundColor: active ? 'rgba(255,107,43,0.08)' : '#1a1530', border: active ? '2px solid #ff6b2b' : '1px solid rgba(255,255,255,0.06)', cursor: 'pointer', outline: 'none', padding: 3, transition: 'all 0.12s ease', boxShadow: active ? '0 0 8px rgba(255,107,43,0.15), inset 0 1px 2px rgba(255,255,255,0.05)' : 'inset 0 1px 3px rgba(0,0,0,0.4)', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
      <canvas ref={canvasRef} width={70} height={40} style={{ width: '100%', height: '100%', display: 'block' }} />
    </button>
  )
}

export function Visualizer({ activePreset, onPresetChange }: VisualizerProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return
    const ctx = canvas.getContext('2d')
    if (!ctx) return
    const w = canvas.width, h = canvas.height
    const curveFn = CURVE_PRESETS[activePreset]
    ctx.clearRect(0, 0, w, h)
    ctx.strokeStyle = 'rgba(255,255,255,0.03)'; ctx.lineWidth = 0.5
    for (let i = 1; i < 4; i++) { ctx.beginPath(); ctx.moveTo(0, (h / 4) * i); ctx.lineTo(w, (h / 4) * i); ctx.stroke() }
    for (let i = 1; i < 8; i++) { ctx.beginPath(); ctx.moveTo((w / 8) * i, 0); ctx.lineTo((w / 8) * i, h); ctx.stroke() }
    ctx.strokeStyle = 'rgba(255,255,255,0.12)'; ctx.lineWidth = 1; ctx.beginPath()
    for (let i = 0; i < w; i++) {
      const x = i / w
      const waveVal = Math.sin(x * Math.PI * 18) * 0.25 + Math.sin(x * Math.PI * 7.3) * 0.18 + Math.sin(x * Math.PI * 31) * 0.06 + Math.sin(x * Math.PI * 53) * 0.04
      const ducking = curveFn(x)
      const amplitude = 0.3 + ducking * 0.5
      const y = h / 2 + waveVal * amplitude * h * 0.45
      if (i === 0) ctx.moveTo(i, y); else ctx.lineTo(i, y)
    }
    ctx.stroke()
    ctx.beginPath()
    for (let i = 0; i < w; i++) {
      const x = i / w
      const waveVal = Math.sin(x * Math.PI * 18) * 0.25 + Math.sin(x * Math.PI * 7.3) * 0.18 + Math.sin(x * Math.PI * 31) * 0.06 + Math.sin(x * Math.PI * 53) * 0.04
      const ducking = curveFn(x)
      const amplitude = 0.3 + ducking * 0.5
      const y = h / 2 - waveVal * amplitude * h * 0.45
      if (i === 0) ctx.moveTo(i, y); else ctx.lineTo(i, y)
    }
    ctx.stroke()
    ctx.strokeStyle = '#ff6b2b'; ctx.lineWidth = 2.5; ctx.shadowColor = 'rgba(255,107,43,0.3)'; ctx.shadowBlur = 6; ctx.beginPath()
    for (let i = 0; i <= w; i++) {
      const x = i / w; const y = h - curveFn(x) * (h - 20) - 10
      if (i === 0) ctx.moveTo(i, y); else ctx.lineTo(i, y)
    }
    ctx.stroke(); ctx.shadowBlur = 0
    ctx.strokeStyle = 'rgba(255,255,255,0.4)'; ctx.lineWidth = 1; ctx.beginPath()
    const playheadX = w * 0.82; ctx.moveTo(playheadX, 0); ctx.lineTo(playheadX, h); ctx.stroke()
    ctx.fillStyle = 'rgba(255,255,255,0.5)'; ctx.fillRect(playheadX - 4, 6, 8, 12)
  }, [activePreset])

  const navigatePreset = useCallback((delta: number) => {
    onPresetChange(Math.max(0, Math.min(CURVE_PRESETS.length - 1, activePreset + delta)))
  }, [activePreset, onPresetChange])

  return (
    <div className="flex flex-col flex-1 min-w-0" style={{ padding: '6px 8px 8px 4px' }}>
      <div className="relative flex-1" style={{ backgroundColor: '#130f24', borderRadius: 6, boxShadow: 'inset 0 2px 8px rgba(0,0,0,0.6), inset 0 -1px 3px rgba(255,255,255,0.03)', overflow: 'hidden' }}>
        <canvas ref={canvasRef} width={800} height={360} style={{ width: '100%', height: '100%', display: 'block' }} />
      </div>
      <div className="flex items-center justify-between mt-1.5" style={{ height: 28 }}>
        <div className="flex items-center gap-1">
          <NavButton onClick={() => navigatePreset(-4)}><ChevronsLeftIcon size={14} /></NavButton>
          <NavButton onClick={() => navigatePreset(-1)}><ChevronLeftIcon size={14} /></NavButton>
        </div>
        <div className="flex items-center gap-2">
          <span className="font-inter font-bold" style={{ fontSize: 10, color: '#6b6880', letterSpacing: '0.05em' }}>carbonated audio</span>
        </div>
        <div className="flex items-center gap-1">
          <NavButton onClick={() => navigatePreset(1)}><ChevronRightIcon size={14} /></NavButton>
          <NavButton onClick={() => navigatePreset(4)}><ChevronsRightIcon size={14} /></NavButton>
        </div>
      </div>
      <div className="flex flex-col gap-1 mt-1">
        <div className="flex gap-1">
          {CURVE_PRESETS.slice(0, 8).map((fn, i) => (<MiniPreview key={i} curveFn={fn} active={activePreset === i} onClick={() => onPresetChange(i)} />))}
        </div>
        <div className="flex gap-1">
          {CURVE_PRESETS.slice(8, 16).map((fn, i) => (<MiniPreview key={i + 8} curveFn={fn} active={activePreset === i + 8} onClick={() => onPresetChange(i + 8)} />))}
        </div>
      </div>
    </div>
  )
}

function NavButton({ onClick, children }: { onClick: () => void; children: React.ReactNode }) {
  return (
    <button onClick={onClick} style={{ width: 28, height: 24, borderRadius: 4, backgroundColor: '#1a1530', border: '1px solid rgba(255,255,255,0.06)', color: '#6b6880', cursor: 'pointer', outline: 'none', display: 'flex', alignItems: 'center', justifyContent: 'center', transition: 'all 0.1s ease', boxShadow: 'inset 0 1px 2px rgba(0,0,0,0.4)' }}>
      {children}
    </button>
  )
}
