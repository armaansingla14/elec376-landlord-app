import React from 'react'
import { useLocation, useNavigate } from 'react-router-dom'

export default function ReportedInbox({ user }) {
  const location = useLocation()
  const navigate = useNavigate()
  const [loading, setLoading] = React.useState(true)
  const [err, setErr] = React.useState('')
  const [reports, setReports] = React.useState([])
  const [selected, setSelected] = React.useState(null)

  React.useEffect(() => {
    // Ensure this page was reached via the Admin button (frontend-level guard)
    if (!location.state || !location.state.fromAdmin) {
      navigate('/admin', { replace: true })
      return
    }

    const token = localStorage.getItem('token')

    // Attempt to fetch reported reviews from backend. If backend not implemented yet,
    // fall back to an empty list so the page still works.
    ;(async () => {
      try {
        const res = await fetch('/api/admin/reported', {
          headers: token ? { 'Authorization': `Bearer ${token}` } : {}
        })
        if (!res.ok) {
          // backend endpoint likely not implemented yet — try frontend sample file as fallback
          try {
            const fallback = await fetch('/reported.json')
            const j2 = await fallback.json().catch(() => ({}))
            setReports(j2.reports || [])
            setErr('Loaded local sample reports (backend endpoint missing).')
          } catch (e2) {
            setReports([])
            setErr('No reports available (backend endpoint missing).')
          }
        } else {
          const j = await res.json().catch(() => ({}))
          setReports(j.reports || [])
        }
      } catch (e) {
        // network error; fallback to frontend public sample if present
        try {
          const fallback = await fetch('/reported.json')
          const j2 = await fallback.json().catch(() => ({}))
          setReports(j2.reports || [])
          setErr('Loaded local sample reports (network to backend failed).')
        } catch (e2) {
          setErr('Failed to reach server; showing local view.')
          setReports([])
        }
      } finally {
        setLoading(false)
      }
    })()
  }, [])

  const [notice, setNotice] = React.useState('')

  const handleAction = (id, action) => {
    // Placeholder only: don't modify data yet. Show a small notice to indicate
    // the feature is not implemented on the backend.
    setNotice(`Action "${action}" is not implemented yet.`)
    setTimeout(() => setNotice(''), 2500)
  }

  const pageStyle = {
    minHeight: '100vh',
    background: 'linear-gradient(135deg, rgba(96, 165, 250, 0.8) 0%, rgba(59, 130, 246, 0.8) 100%)',
    padding: '120px 24px 40px'
  }

  const containerStyle = {
    maxWidth: '1100px',
    margin: '0 auto',
    display: 'grid',
    gridTemplateColumns: '320px 1fr',
    gap: '20px'
  }

  const inboxStyle = {
    backgroundColor: 'rgba(255,255,255,0.06)',
    borderRadius: 12,
    padding: 12,
    border: '1px solid rgba(255,255,255,0.08)',
    height: '70vh',
    overflowY: 'auto'
  }

  const itemStyle = (isSelected) => ({
    background: isSelected ? 'rgba(255,255,255,0.12)' : 'transparent',
    padding: 12,
    borderRadius: 8,
    marginBottom: 8,
    cursor: 'pointer',
    border: isSelected ? '1px solid rgba(255,255,255,0.14)' : '1px solid transparent'
  })

  const detailStyle = {
    backgroundColor: 'rgba(255,255,255,0.06)',
    borderRadius: 12,
    padding: 20,
    border: '1px solid rgba(255,255,255,0.08)',
    height: '70vh',
    overflowY: 'auto',
    color: '#fff'
  }

  return (
    <div style={pageStyle}>
      <div style={containerStyle}>
        <div style={{gridColumn: '1 / 2'}}>
          <h3 style={{color:'#fff', marginTop:0}}>Reported Reviews</h3>
          <div style={inboxStyle}>
            {loading ? (
              <div style={{color:'#e5e7eb'}}>Loading…</div>
            ) : err ? (
              <div style={{color:'#fecaca'}}>{err}</div>
            ) : reports.length === 0 ? (
              <div style={{color:'#e5e7eb', padding:16}}>No reported reviews yet. When reports arrive they will appear here.</div>
            ) : (
              reports.map((r) => (
                <div key={r.id} style={itemStyle(selected && selected.id === r.id)} onClick={() => setSelected(r)}>
                  <div style={{fontWeight:700, color:'#fff'}}>{r.title || 'Untitled review'}</div>
                  <div style={{fontSize:12, opacity:0.85, color:'#e5e7eb'}}>By: {r.reported_by || 'Unknown'}</div>
                  <div style={{fontSize:12, opacity:0.75, color:'#c7d2fe', marginTop:6}}>Review id: {r.review_id}</div>
                </div>
              ))
            )}
          </div>
        </div>

        <div style={{gridColumn: '2 / 3'}}>
          <h3 style={{color:'#fff', marginTop:0}}>Details</h3>
          <div style={detailStyle}>
            {notice && <div style={{background:'rgba(255,255,255,0.06)', padding:8, borderRadius:6, marginBottom:12, color:'#fff'}}>{notice}</div>}
            {!selected ? (
              <div style={{color:'#e5e7eb'}}>Select a report from the left to see details and take action.</div>
            ) : (
              <>
                <div style={{display:'flex', justifyContent:'space-between', alignItems:'flex-start', gap:12}}>
                  <div>
                    <h2 style={{margin:'0 0 8px 0'}}>{selected.title || 'Untitled review'}</h2>
                    <div style={{fontSize:13, opacity:0.85}}>Reported by: {selected.reported_by || 'Unknown'}</div>
                    <div style={{fontSize:12, opacity:0.7, marginTop:6}}>Review id: {selected.review_id}</div>
                  </div>
                </div>

                <div style={{marginTop:16, padding:12, background:'rgba(255,255,255,0.02)', borderRadius:8}}>
                  <div style={{whiteSpace:'pre-wrap', color:'#f8fafc'}}>{selected.review || 'No review text available.'}</div>
                </div>

                <div style={{marginTop:18, display:'flex', gap:12}}>
                  <button onClick={() => handleAction(selected.id, 'approve')} style={{padding:'8px 14px', borderRadius:8, background:'#10b981', color:'#fff', border:'none', cursor:'pointer'}}>Approve</button>
                  <button onClick={() => handleAction(selected.id, 'deny')} style={{padding:'8px 14px', borderRadius:8, background:'#ef4444', color:'#fff', border:'none', cursor:'pointer'}}>Deny</button>
                </div>
              </>
            )}
          </div>
        </div>
      </div>
    </div>
  )
}
