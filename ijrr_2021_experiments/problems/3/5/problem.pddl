(define (problem ijrr_5)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street28 street57 - street
		building12 building15 building25 building31 - building
	)
	(:init
		(atLocation survivor0 street28)
		(atLocation survivor1 building25)
		(onFire building12)
		(onFire building15)
		(underRubble street57)
		(underRubble building31)
		(needsRepair building12)
		(needsRepair building15)
		(needsRepair building31)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street57))
		(not (needsRepair building12))
		(not (needsRepair building15))
		(not (needsRepair building31))
	))
	(:metric minimize total-time)
)
