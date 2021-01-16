(define (problem ijrr_19)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street24 street28 - street
		building12 building15 building22 building31 - building
	)
	(:init
		(atLocation survivor0 street24)
		(atLocation survivor1 building12)
		(onFire building22)
		(onFire building31)
		(underRubble street28)
		(underRubble building15)
		(needsRepair building15)
		(needsRepair building22)
		(needsRepair building31)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street28))
		(not (needsRepair building15))
		(not (needsRepair building22))
		(not (needsRepair building31))
	))
	(:metric minimize total-time)
)
